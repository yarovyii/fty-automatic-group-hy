#pragma once
#include "common/message.h"

namespace fty::agroup {

static constexpr const char* AgentName = "automatic_group_rest";


inline fty::Message message(const std::string& subj)
{
    fty::Message msg;
    msg.meta.to      = "automatic-group";
    msg.meta.replyTo = AgentName;
    msg.meta.subject = subj;
    msg.meta.from    = AgentName;
    return msg;
}

}
