#include "save.h"
#include "common/group.h"
#include "common/logger.h"
#include "common/srr.h"
#include "utils.h"
#include <list>
#include <pack/serialization.h>

namespace fty::job::srr {

dto::srr::SaveResponse save(const dto::srr::SaveQuery& query)
{
    using namespace dto;
    using namespace dto::srr;

    logDebug("Saving automatic groups");

    std::map<FeatureName, FeatureAndStatus> mapFeaturesData;

    for (const auto& featureName : query.features()) {
        FeatureAndStatus fs1;
        Feature&         f1 = *(fs1.mutable_feature());

        if (featureName == common::srr::FeatureName) {
            f1.set_version(common::srr::ActiveVersion);
            try {
                common::srr::SrrGroupObj payload;

                auto idList = utils::list();
                if (!idList) {
                    throw std::runtime_error(idList.error());
                }

                for (const auto& id : *idList) {

                    auto groupInfo = utils::read(id);

                    if (!groupInfo) {
                        throw std::runtime_error(groupInfo.error());
                    }

                    payload.groupList.append(*groupInfo);
                }

                f1.set_data(*pack::json::serialize(payload));
                fs1.mutable_status()->set_status(Status::SUCCESS);
            } catch (std::exception& e) {
                fs1.mutable_status()->set_status(Status::FAILED);
                fs1.mutable_status()->set_error(e.what());
            }

        } else {
            fs1.mutable_status()->set_status(Status::FAILED);
            fs1.mutable_status()->set_error("Feature is not supported");
        }

        mapFeaturesData[featureName] = fs1;
    }

    return (createSaveResponse(mapFeaturesData, common::srr::ActiveVersion)).save();
}

} // namespace fty::job::srr
