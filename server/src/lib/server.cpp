#include "server.h"
#include "common/logger.h"
#include "common/message-bus.h"
#include "common/srr.h"
#include "config.h"
#include "daemon.h"
#include "jobs/create.h"
#include "jobs/list.h"
#include "jobs/read.h"
#include "jobs/remove.h"
#include "jobs/resolve.h"
#include "jobs/srr/reset.h"
#include "jobs/srr/restore.h"
#include "jobs/srr/save.h"
#include "jobs/update.h"
#include <asset/db.h>
#include <fty_common_messagebus.h>
#include <fty_srr_dto.h>

namespace fty {

Expected<void> Server::run()
{
    m_stopSlot.connect(Daemon::instance().stopEvent);
    m_loadConfigSlot.connect(Daemon::instance().loadConfigEvent);

    if (auto res = m_bus.init(Config::instance().actorName); !res) {
        return unexpected(res.error());
    }

    if (auto sub = m_bus.subscribe(Channel, &Server::process, this); !sub) {
        return unexpected(sub.error());
    }

    m_srrProcessor.saveHandler    = std::bind(&job::srr::save, std::placeholders::_1);
    m_srrProcessor.restoreHandler = std::bind(&job::srr::restore, std::placeholders::_1);
    m_srrProcessor.resetHandler   = std::bind(&job::srr::reset, std::placeholders::_1);

    if (auto sub = m_bus.subscribeLegacy(common::srr::Channel, &Server::srrProcess, this); !sub) {
        return unexpected(sub.error());
    }

    return {};
}

void Server::wait()
{
    stop.wait();
}

void Server::doStop()
{
    stop();
}

void Server::reloadConfig()
{
    Config::instance().reload();
}

void Server::process(const Message& msg)
{
    logDebug("Automatic group: got message {}", msg.dump());
    if (msg.meta.subject == commands::create::Subject) {
        m_pool.pushWorker<job::Create>(msg, m_bus);
    } else if (msg.meta.subject == commands::update::Subject) {
        m_pool.pushWorker<job::Update>(msg, m_bus);
    } else if (msg.meta.subject == commands::remove::Subject) {
        m_pool.pushWorker<job::Remove>(msg, m_bus);
    } else if (msg.meta.subject == commands::list::Subject) {
        m_pool.pushWorker<job::List>(msg, m_bus);
    } else if (msg.meta.subject == commands::read::Subject) {
        m_pool.pushWorker<job::Read>(msg, m_bus);
    } else if (msg.meta.subject == commands::resolve::Subject) {
        m_pool.pushWorker<job::Resolve>(msg, m_bus);
    }
}

void Server::srrProcess(const messagebus::Message& msg)
{
    using namespace dto;
    using namespace dto::srr;

    logDebug("Handle SRR message");

    try {
        // Get request
        UserData data = msg.userData();
        Query    query;
        data >> query;

        messagebus::UserData         respData;
        std::unique_lock<std::mutex> lock(m_srrLock);
        respData << (m_srrProcessor.processQuery(query));
        lock.unlock();
        const auto subject = msg.metaData().at(messagebus::Message::SUBJECT);
        const auto replyTo = msg.metaData().at(messagebus::Message::REPLY_TO);

        messagebus::Message resp;
        resp.metaData().emplace(messagebus::Message::SUBJECT, subject);
        resp.metaData().emplace(messagebus::Message::STATUS, messagebus::STATUS_OK);

        resp.userData() = respData;

        if (auto ans = m_bus.replyLegacy(replyTo, msg, resp); !ans) {
            throw std::runtime_error("Sending reply failed");
        }
    } catch (std::exception& e) {
        logError("Unexpected error {}", e.what());
    } catch (...) {
        logError("Unexpected error: unknown");
    }
}

void Server::shutdown()
{
    stop();
    m_pool.stop();
}

} // namespace fty
