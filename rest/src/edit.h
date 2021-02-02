#pragma once
#include <fty/rest/runner.h>

namespace fty::agroup {

class Edit : public rest::Runner
{
public:
    INIT_REST("agroup/edit");

public:
    unsigned run() override;

private:
    // clang-format off
    Permissions m_permissions = {
        { rest::User::Profile::Admin, rest::Access::Update }
    };
    // clang-format on
};

} // namespace fty::agroup
