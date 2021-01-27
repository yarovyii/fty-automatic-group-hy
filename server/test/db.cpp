#include "lib/storage.h"
#include <catch2/catch.hpp>


TEST_CASE("DB")
{
    fty::Group group;
    group.name     = "group 1";
    group.rules.op = fty::Group::LogicalOp::And;
    auto& cond     = group.rules.conditions.append();
    cond.field     = "name";
    cond.op        = fty::Group::ConditionOp::Contains;
    cond.entity    = "value";

    auto ins = fty::Storage::save(group);
    REQUIRE(ins);

    SECTION("keys")
    {
        auto keys = fty::Storage::names();
        REQUIRE(!keys.empty());
        CHECK(keys[0] == "group 1");
    }

    SECTION("byName")
    {
        auto it = fty::Storage::byName("group 1");
        REQUIRE(it);
        REQUIRE(*it == *ins);
    }

    SECTION("update")
    {
        ins->name = "modified group";
        fty::Storage::save(*ins);
        auto it = fty::Storage::byName("modified group");
        REQUIRE(it);
        REQUIRE(*it == *ins);
    }

    fty::Storage::remove(ins->id);
}
