#include <catch2/catch.hpp>
#include "lib/jobs/list.h"
#include "lib/jobs/create.h"
#include "db.h"


TEST_CASE("List")
{
    fty::job::List lst;
    fty::commands::list::Out out;

    REQUIRE(fty::GroupsDB::init());

    // Initial size should be zero
    REQUIRE_NOTHROW(lst.run(out));
    CHECK(out.size() == 0);

    // Create any group
    fty::Group group;
    group.name          = "My group";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    {
        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "srv";
        cond.field = fty::Group::Fields::Name;
        cond.op    = fty::Group::ConditionOp::Contains;
    }

    fty::job::Create create;
    fty::Group created;
    REQUIRE_NOTHROW(create.run(group, created));
    CHECK(created.hasValue());

    REQUIRE_NOTHROW(lst.run(out));
    CHECK(out.size() == 1);
    CHECK(out[0].id == created.id);
    CHECK(out[0].name == created.name);
}
