#include "resolve.h"
#include "common/commands.h"
#include "common/message-bus.h"
#include "group-rest.h"
#include <fty/rest/component.h>
#include <asset/asset-db.h>
#include <asset/json.h>

namespace fty::agroup {

unsigned Resolve::run()
{
    rest::User user(m_request);
    if (auto ret = checkPermissions(user.profile(), m_permissions); !ret) {
        throw rest::Error(ret.error());
    }

    auto strIdPrt = m_request.queryArg<std::string>("id");
    if (!strIdPrt) {
        throw rest::errors::RequestParamRequired("id");
    }

    fty::MessageBus bus;
    if (auto res = bus.init(AgentName); !res) {
        throw rest::errors::Internal(res.error());
    }

    fty::Message msg = message(fty::commands::resolve::Subject);
    msg.userData.setString(*strIdPrt);

    auto ret = bus.send(fty::Channel, msg);
    if (!ret) {
        throw rest::errors::Internal(ret.error());
    }

    auto info = ret->userData.decode<fty::commands::resolve::Out>();
    if (!info) {
        throw rest::errors::Internal(info.error());
    }

    pack::StringList out;
    for(const auto& it: *info) {
        if (auto res = asset::db::selectAssetElementByName(it.name)) {
        } else {
            throw rest::errors::Internal(res.error());
        }
    }

    m_reply << *pack::json::serialize(*info);

    return HTTP_OK;
}

} // namespace fty::agroup

registerHandler(fty::agroup::Resolve)
