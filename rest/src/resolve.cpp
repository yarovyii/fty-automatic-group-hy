#include "resolve.h"
#include "common/commands.h"
#include "common/message-bus.h"
#include "group-rest.h"
#include <fty/rest/component.h>
#include <asset/json.h>
#include <fty/split.h>

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

    std::vector<std::string> out;
    for(const auto& it: *info) {
        std::string json = asset::getJsonAsset(fty::convert<uint32_t>(it.id.value()));
        if (json.empty()) {
            throw rest::errors::Internal("Cannot build asset information");
        }
        out.push_back(json);
    }

    m_reply << "[" << fty::implode(out, ", ") << "]";

    return HTTP_OK;
}

} // namespace fty::agroup

registerHandler(fty::agroup::Resolve)
