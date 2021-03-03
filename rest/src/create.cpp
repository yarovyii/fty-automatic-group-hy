#include "create.h"
#include <fty/rest/component.h>
#include "common/message-bus.h"
#include "group-rest.h"
#include "common/commands.h"

namespace fty::agroup {

unsigned Create::run()
{
    rest::User user(m_request);
    if (auto ret = checkPermissions(user.profile(), m_permissions); !ret) {
        throw rest::Error(ret.error());
    }

    std::string json = m_request.body();
    if (json.empty()) {
        throw rest::errors::BadInput("Group payload is empty");
    }

    fty::MessageBus bus;
    if (auto res = bus.init(AgentName); !res) {
        throw rest::errors::Internal(res.error());
    }

    auto msg = message(commands::create::Subject);
    msg.userData.append(json);

    if (auto ret = bus.send(fty::Channel, msg)) {
        commands::create::Out out;
        if (auto info = pack::json::deserialize(ret->userData[0], out)) {
            m_reply << *pack::json::serialize(out);
            return HTTP_OK;
        } else {
            throw rest::errors::Internal(info.error());
        }
    } else {
        throw rest::errors::Internal(ret.error());
    }
}

}

registerHandler(fty::agroup::Create)
