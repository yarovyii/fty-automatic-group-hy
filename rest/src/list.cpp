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

    m_reply << *pack::json::serialize(*info);

    return HTTP_OK;
}

} // namespace fty::asset

registerHandler(fty::agroup::List)
