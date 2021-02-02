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
    msg.userData.setString(json);

    auto ret = bus.send(fty::Channel, msg);
    if (!ret) {
        throw rest::errors::Internal(ret.error());
    }

    return HTTP_OK;
}

}

registerHandler(fty::agroup::Create)
