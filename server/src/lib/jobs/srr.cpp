#include "srr.h"
#include "common/logger.h"
#include "common/srr_common.h"
#include "lib/config.h"
#include <functional>
#include <map>

namespace fty::job::srr {

Expected<void> SrrHandler::run()
{
    if (m_clientName.empty()) {
        return unexpected("Client name is empty");
    }

    try {
        m_srrBus.reset(messagebus::MlmMessageBus(MessageBus::endpoint, m_clientName));

        m_srrBus->connect();

        m_srrProcessor.saveHandler    = std::bind(&SrrHandler::save, this, std::placeholders::_1);
        m_srrProcessor.restoreHandler = std::bind(&SrrHandler::restore, this, std::placeholders::_1);
        m_srrProcessor.resetHandler   = std::bind(&SrrHandler::reset, this, std::placeholders::_1);

        m_srrBus->receive(fty::SrrChannel, [&](messagebus::Message m) {
            this->onMessage(m);
        });
    } catch (const std::exception& e) {
        return unexpected(e.what());
    }

    m_busName = m_clientName + "-bus";

    fty::MessageBus bus;
    if (auto res = bus.init(m_busName); !res) {
        return unexpected(res.error());
    }

    return {};
}

void SrrHandler::onMessage(const messagebus::Message& msg)
{
    using namespace dto;
    using namespace dto::srr;

    logDebug("Handle SRR message");

    try {
        // Get request
        UserData data = msg.userData();
        Query    query;
        data >> query;

        messagebus::UserData respData;
        respData << (m_srrProcessor.processQuery(query));

        const auto subject = msg.metaData().at(messagebus::Message::SUBJECT);
        const auto from    = msg.metaData().at(messagebus::Message::FROM);
        const auto to      = msg.metaData().at(messagebus::Message::REPLY_TO);
        const auto corrId  = msg.metaData().at(messagebus::Message::CORRELATION_ID);

        messagebus::Message resp;
        resp.metaData().emplace(messagebus::Message::SUBJECT, subject);
        resp.metaData().emplace(messagebus::Message::FROM, from);
        resp.metaData().emplace(messagebus::Message::TO, to);
        resp.metaData().emplace(messagebus::Message::CORRELATION_ID, corrId);
        resp.metaData().emplace(messagebus::Message::STATUS, messagebus::STATUS_OK);

        resp.userData() = respData;

        m_srrBus->sendReply(msg.metaData().find(messagebus::Message::REPLY_TO)->second, resp);
    } catch (std::exception& e) {
        logError("Unexpected error {}", e.what());
    } catch (...) {
        logError("Unexpected error: unknown");
    }
}

dto::srr::SaveResponse SrrHandler::save(const dto::srr::SaveQuery& query)
{
    using namespace dto;
    using namespace dto::srr;

    logDebug("Saving automatic groups");

    std::map<FeatureName, FeatureAndStatus> mapFeaturesData;

    for (const auto& featureName : query.features()) {
        FeatureAndStatus fs1;
        Feature&         f1 = *(fs1.mutable_feature());

        if (featureName == SrrFeatureName) {
            f1.set_version(SrrActiveVersion);
            try {
                pack::ObjectList<fty::Group> payload;

                std::unique_lock<std::mutex> lock(m_srrLock);

                auto groupList = list();
                if (!groupList) {
                    throw std::runtime_error(groupList.error());
                }

                for (const auto& group : *groupList) {
                    auto groupInfo = read(group.id);

                    if (!groupInfo) {
                        throw std::runtime_error(groupInfo.error());
                    }

                    payload.append(*groupInfo);
                }

                f1.set_data(*pack::json::serialize(payload));
                fs1.mutable_status()->set_status(Status::SUCCESS);
            } catch (std::exception& e) {
                fs1.mutable_status()->set_status(Status::FAILED);
                fs1.mutable_status()->set_error(e.what());
            }

        } else {
            fs1.mutable_status()->set_status(Status::FAILED);
            fs1.mutable_status()->set_error("Feature is not supported!");
        }

        mapFeaturesData[featureName] = fs1;
    }

    return (createSaveResponse(mapFeaturesData, SrrActiveVersion)).save();
}

dto::srr::RestoreResponse SrrHandler::restore(const dto::srr::RestoreQuery& query)
{
    using namespace dto;
    using namespace dto::srr;

    logDebug("Restoring automatic groups");

    std::map<FeatureName, FeatureStatus> mapStatus;

    for (const auto& item : query.map_features_data()) {
        const FeatureName& featureName = item.first;
        const Feature&     feature     = item.second;

        FeatureStatus featureStatus;

        if (featureName == SrrFeatureName) {
            try {
                std::unique_lock<std::mutex> lock(m_srrLock);

                auto groupList = list();
                if (!groupList) {
                    throw std::runtime_error(groupList.error());
                }

                // check if group list is empty
                if (!groupList->empty()) {
                    throw std::runtime_error("Restore operation cannot be completed: there are existing groups");
                }

                // get data to restore
                pack::ObjectList<fty::Group> groups;
                pack::json::deserialize(feature.data(), groups);

                // restore groups
                for (const auto& group : groups) {
                    auto ret = create(group);
                    if (!ret) {
                        throw std::runtime_error(ret.error());
                    }
                }

                featureStatus.set_status(Status::SUCCESS);
            } catch (std::exception& e) {

                featureStatus.set_status(Status::FAILED);
                featureStatus.set_error(e.what());
            }

        } else {
            featureStatus.set_status(Status::FAILED);
            featureStatus.set_error("Feature is not supported!");
        }

        mapStatus[featureName] = featureStatus;
    }

    std::map<FeatureName, FeatureAndStatus> mapFeaturesData;

    return (createRestoreResponse(mapStatus)).restore();
}

dto::srr::ResetResponse SrrHandler::reset(const dto::srr::ResetQuery& /* query */)
{
    using namespace dto;
    using namespace dto::srr;

    logDebug("Reset automatic groups");

    std::map<FeatureName, FeatureStatus> mapStatus;

    const FeatureName& featureName = SrrFeatureName;
    FeatureStatus      featureStatus;

    try {
        if (auto ret = clear(); !ret) {
            throw std::runtime_error(ret.error());
        }
        featureStatus.set_status(Status::SUCCESS);
    } catch (std::exception& ex) {
        featureStatus.set_status(Status::FAILED);
        featureStatus.set_error("Reset of automatic groups failed");
    }

    mapStatus[featureName] = featureStatus;

    return (createResetResponse(mapStatus)).reset();
}

fty::Expected<fty::commands::list::Out> SrrHandler::list()
{
    fty::Message msg;
    msg.meta.to      = fty::Config::instance().actorName;
    msg.meta.subject = fty::commands::list::Subject;
    msg.meta.from    = m_busName;

    auto ret = m_bus.send(fty::Channel, msg);
    if (!ret) {
        return unexpected(ret.error());
    }

    auto info = ret->userData.decode<fty::commands::list::Out>();
    if (!info) {
        return unexpected(info.error());
    }

    return *info;
}

fty::Expected<fty::commands::read::Out> SrrHandler::read(const uint64_t id)
{
    fty::Message msg;
    msg.meta.to      = fty::Config::instance().actorName;
    msg.meta.subject = fty::commands::read::Subject;
    msg.meta.from    = m_busName;

    fty::commands::read::In in;
    in.id = id;

    msg.userData.setString(*pack::json::serialize(in));

    auto ret = m_bus.send(fty::Channel, msg);
    if (!ret) {
        return unexpected(ret.error());
    }

    auto info = ret->userData.decode<fty::commands::read::Out>();
    if (!info) {
        return unexpected(info.error());
    }
    return *info;
}

fty::Expected<void> SrrHandler::create(const fty::Group& group)
{
    fty::Message msg;
    msg.meta.to      = fty::Config::instance().actorName;
    msg.meta.subject = fty::commands::create::Subject;
    msg.meta.from    = m_busName;

    msg.userData.setString(*pack::json::serialize(group));

    auto ret = m_bus.send(fty::Channel, msg);
    if (!ret) {
        return unexpected(ret.error());
    }

    if (ret->meta.status == fty::Message::Status::Ok) {
        auto info = ret->userData.decode<fty::commands::create::Out>();
        if (!info) {
            return unexpected(info.error());
        }

        if (info->id > 0) {
            logDebug("Created new group {} with ID {}", info->name.value(), info->id.value());
        } else {
            return unexpected("Create new group failed: id");
        }
    } else {
        return unexpected("Create new group failed: status");
    }

    return {};
}

fty::Expected<void> SrrHandler::remove(const uint64_t id)
{
    fty::Message msg;
    msg.meta.to      = fty::Config::instance().actorName;
    msg.meta.subject = fty::commands::remove::Subject;
    msg.meta.from    = m_busName;

    fty::commands::remove::In in;
    in.append(id);

    msg.userData.setString(*pack::json::serialize(in));
    auto ret = m_bus.send(fty::Channel, msg);
    if (!ret) {
        return unexpected(ret.error());
    }

    return {};
}

fty::Expected<void> SrrHandler::clear()
{
    auto groupList = list();
    if (!groupList) {
        return unexpected(groupList.error());
    }

    for (const auto& group : *groupList) {
        auto ret = remove(group.id);
        if (!ret) {
            return unexpected(ret.error());
        }
    }

    return {};
}

} // namespace fty::job::srr
