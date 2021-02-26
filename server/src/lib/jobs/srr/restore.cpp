#include "restore.h"
#include "common/group.h"
#include "common/logger.h"
#include "common/srr.h"
#include "utils.h"
#include <list>
#include <pack/serialization.h>

namespace fty::job::srr {

dto::srr::RestoreResponse restore(const dto::srr::RestoreQuery& query)
{
    using namespace dto;
    using namespace dto::srr;

    logDebug("Restoring automatic groups");

    std::map<FeatureName, FeatureStatus> mapStatus;

    for (const auto& item : query.map_features_data()) {
        const FeatureName& featureName = item.first;
        const Feature&     feature     = item.second;

        FeatureStatus featureStatus;

        if (featureName == common::srr::FeatureName) {
            try {
                auto groupList = utils::list();
                if (!groupList) {
                    throw std::runtime_error(groupList.error());
                }

                // check if group list is empty
                if (!groupList->empty()) {
                    throw std::runtime_error("Restore operation cannot be completed: there are existing groups");
                }

                // get data to restore
                pack::ObjectList<Group> srrRestoreData;
                pack::json::deserialize(feature.data(), srrRestoreData);

                // restore groups
                for (auto& group : srrRestoreData) {
                    logDebug("Restoring group {}", group.name.value());
                    // group must have no ID at insertion
                    group.id.clear();

                    auto ret = utils::create(group);
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

} // namespace fty::job::srr
