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

    commands::list::Out listOut;
    auto info = pack::json::deserialize(ret->userData[0], listOut);
    if (!info) {
        throw rest::errors::Internal(info.error());
    }

    if (listOut.size()) {
        pack::ObjectList<fty::commands::read::Out> out;
        for(const auto& it: listOut) {
            fty::Message readMsg = message(fty::commands::read::Subject);

            fty::commands::read::In in;
            in.id = it.id;

            readMsg.setData(*pack::json::serialize(in));

            auto readRet = bus.send(fty::Channel, readMsg);
            if (!readRet) {
                throw rest::errors::Internal(readRet.error());
            }

            commands::read::Out group;
            auto r = pack::json::deserialize(readRet->userData[0], group);
            if (!r) {
                throw rest::errors::Internal(r.error());
            }
            out.append(group);
        }
        m_reply << *pack::json::serialize(out);
    } else {
        m_reply << "[]";
    }

    return HTTP_OK;
}

} // namespace fty::asset

registerHandler(fty::agroup::List)
