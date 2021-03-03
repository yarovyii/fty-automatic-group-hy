#include "edit.h"
#include <fty/rest/component.h>
#include "common/message-bus.h"
#include "group-rest.h"
#include "common/commands.h"

namespace fty::agroup {

unsigned Edit::run()
{
    rest::User user(m_request);
    if (auto ret = checkPermissions(user.profile(), m_permissions); !ret) {
        throw rest::Error(ret.error());
    }

    auto strIdPrt = m_request.queryArg<std::string>("id");
    if (!strIdPrt) {
        throw rest::errors::RequestParamRequired("id");
    }

    std::string json = m_request.body();
    if (json.empty()) {
        throw rest::errors::BadInput("Group payload is empty");
    }

    Group group;
    if (auto ret = pack::json::deserialize(json, group); !ret) {
        throw rest::errors::Internal(ret.error());
    }

    group.id = fty::convert<uint64_t>(*strIdPrt);

    fty::MessageBus bus;
    if (auto res = bus.init(AgentName); !res) {
        throw rest::errors::Internal(res.error());
    }

    auto msg = message(commands::update::Subject);
    msg.userData.append(*pack::json::serialize(group));

    if (auto ret = bus.send(fty::Channel, msg)) {
        commands::update::Out out;
        if (auto info = pack::json::deserialize(ret->userData[0], out)) {
            m_reply << *pack::json::serialize(out);
            return HTTP_OK;
        } else {
            throw rest::errors::Internal(info.error());
        }
    } else {
        throw rest::errors::Internal(ret.error());
    }

    return HTTP_OK;
}

}

registerHandler(fty::agroup::Edit)
