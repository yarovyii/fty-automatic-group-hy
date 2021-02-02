#pragma once
#include <fty/rest/runner.h>

namespace fty::agroup {

class Create: public rest::Runner
{
public:
    INIT_REST("agroup/create");

public:
    unsigned run() override;
private:
    // clang-format off
    Permissions m_permissions = {
        { rest::User::Profile::Admin, rest::Access::Create }
    };
    // clang-format on
};

}

