#pragma once

#include "group.h"
#include <pack/pack.h>


namespace fty::common::srr {

static constexpr const char* Channel       = "FTY.Q.GROUP.SRR";
static constexpr const char* FeatureName   = "automatic-groups";
static constexpr const char* ActiveVersion = "1.0";

struct SrrGroupObj : public pack::Node
{
    pack::ObjectList<Group> m_groupList = FIELD("groupList");
    using pack::Node::Node;
    META(SrrGroupObj, m_groupList);
};

} // namespace fty::common::srr
