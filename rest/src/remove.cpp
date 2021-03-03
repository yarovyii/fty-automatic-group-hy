#include "remove.h"
#include <fty/rest/component.h>
#include "common/message-bus.h"
#include "group-rest.h"
#include "common/commands.h"

namespace fty::agroup {

unsigned Remove::run()
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

    auto msg = message(commands::remove::Subject);

    fty::commands::remove::In in;
    in.append(fty::convert<uint64_t>(*strIdPrt));

    msg.userData.append(*pack::json::serialize(in));

    auto ret = bus.send(fty::Channel, msg);
    if (!ret) {
        throw rest::errors::Internal(ret.error());
    }

    commands::remove::Out out;
    auto info = pack::json::deserialize(ret->userData[0], out);
    if (!info) {
        throw rest::errors::Internal(info.error());
    }

    if (out.size()) {
        m_reply << *pack::json::serialize(out);
    } else {
        m_reply << "[]";
    }

    return HTTP_OK;
}

}

registerHandler(fty::agroup::Remove)
