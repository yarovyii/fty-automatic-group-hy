#include "resolve.h"
#include "common/commands.h"
#include "common/message-bus.h"
#include "group-rest.h"
#include <asset/json.h>
#include <fty/rest/component.h>
#include <fty/split.h>

namespace fty::agroup {

unsigned Resolve::run()
{
    rest::User user(m_request);
    if (auto ret = checkPermissions(user.profile(), m_permissions); !ret) {
        throw rest::Error(ret.error());
    }

    auto        strIdPrt = m_request.queryArg<std::string>("id");
    std::string jsonBody;

    if (!strIdPrt) {
        jsonBody = m_request.body();
        if (jsonBody.empty()) {
            throw rest::errors::RequestParamRequired("id or rules");
        }
    }

    fty::MessageBus bus;
    if (auto res = bus.init(AgentName); !res) {
        throw rest::errors::Internal(res.error());
    }

    fty::Message msg = message(fty::commands::resolve::Subject);

    fty::commands::resolve::In in;
    if (jsonBody.empty()) {
        in.id = fty::convert<uint16_t>(*strIdPrt);
    } else if (auto ret = pack::yaml::deserialize(jsonBody, in); !ret) {
        throw rest::errors::Internal(ret.error());
    }

    msg.setData(*pack::json::serialize(in));

    auto ret = bus.send(fty::Channel, msg);
    if (!ret) {
        throw rest::errors::Internal(ret.error());
    }

    commands::resolve::Out data;
    auto                   info = pack::json::deserialize(ret->userData[0], data);
    if (!info) {
        throw rest::errors::Internal(info.error());
    }

    std::vector<std::string> out;
    for (const auto& it : data) {
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
