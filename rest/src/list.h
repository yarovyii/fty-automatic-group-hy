#pragma once
#include <fty/rest/runner.h>

namespace fty::agroup {

class List: public rest::Runner
{
public:
    INIT_REST("agroup/list");

public:
    unsigned run() override;

private:
    // clang-format off
    Permissions m_permissions = {
        { rest::User::Profile::Admin,     rest::Access::Read },
        { rest::User::Profile::Dashboard, rest::Access::Read }
    };
    // clang-format on
};

}
