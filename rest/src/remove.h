#pragma once
#include <fty/rest/runner.h>

namespace fty::agroup {

class Remove: public rest::Runner
{
public:
    INIT_REST("agroup/remove");

public:
    unsigned run() override;

private:
    // clang-format off
    Permissions m_permissions = {
        { rest::User::Profile::Admin, rest::Access::Delete }
    };
    // clang-format on
};

}
