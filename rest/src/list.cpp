#include "list.h"
#include <fty/rest/audit-log.h>
#include <fty/rest/component.h>
#include <fty_common_asset_types.h>
#include "common/commands.h"
#include "common/message-bus.h"
#include "group-rest.h"

namespace fty::agroup {

unsigned List::run()
{
    rest::User user(m_request);
    if (auto ret = checkPermissions(user.profile(), m_permissions); !ret) {
        throw rest::Error(ret.error());
    }

    fty::MessageBus bus;
    if (auto res = bus.init(AgentName); !res) {
        throw rest::errors::Internal(res.error());
    }

    fty::Message msg = message(fty::commands::list::Subject);

    auto ret = bus.send(fty::Channel, msg);
    if (!ret) {
        throw rest::errors::Internal(ret.error());
    }

    auto info = ret->userData.decode<fty::commands::list::Out>();
    if (!info) {
        throw rest::errors::Internal(info.error());
    }

    if (info->size()) {
        pack::ObjectList<fty::commands::read::Out> out;
        for(const auto& it: *info) {
            fty::Message readMsg = message(fty::commands::read::Subject);

            fty::commands::read::In in;
            in.id = it.id;

            readMsg.userData.setString(*pack::json::serialize(in));

            auto readRet = bus.send(fty::Channel, readMsg);
            if (!readRet) {
                throw rest::errors::Internal(readRet.error());
            }

            auto group = readRet->userData.decode<fty::commands::read::Out>();
            if (!group) {
                throw rest::errors::Internal(group.error());
            }
            out.append(*group);
        }
        m_reply << *pack::json::serialize(out);
    } else {
        m_reply << "[]";
    }

    return HTTP_OK;
}

} // namespace fty::asset

registerHandler(fty::agroup::List)
