#include "reset.h"
#include "common/group.h"
#include "common/logger.h"
#include "common/srr.h"
#include "utils.h"
#include <list>
#include <pack/serialization.h>

namespace fty::job::srr {

dto::srr::ResetResponse reset(const dto::srr::ResetQuery& /* query */)
{
    using namespace dto;
    using namespace dto::srr;

    logDebug("Reset automatic groups");

    std::map<FeatureName, FeatureStatus> mapStatus;

    const FeatureName& featureName = common::srr::FeatureName;
    FeatureStatus      featureStatus;

    try {
        utils::clear();
        featureStatus.set_status(Status::SUCCESS);
    } catch (std::exception& ex) {
        featureStatus.set_status(Status::FAILED);
        featureStatus.set_error("Reset of automatic groups failed");
    }

    mapStatus[featureName] = featureStatus;

    return (createResetResponse(mapStatus)).reset();
}

} // namespace fty::job::srr
