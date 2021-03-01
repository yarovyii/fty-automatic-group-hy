#include "lib/storage.h"
#include <catch2/catch.hpp>


TEST_CASE("Storage")
{
    fty::Group group;
    group.name          = "group 1";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    {
        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.field = fty::Group::Fields::Name;
        cond.op    = fty::Group::ConditionOp::Contains;
        cond.value = "value";
    }

    {
        auto& var     = group.rules.conditions.append();
        auto& rules   = var.reset<fty::Group::Rules>();
        rules.groupOp = fty::Group::LogicalOp::Or;
        auto& var1    = rules.conditions.append();
        auto& cond    = var1.reset<fty::Group::Condition>();
        cond.field    = fty::Group::Fields::Name;
        cond.op       = fty::Group::ConditionOp::Contains;
        cond.value    = "value";
    }

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
    CHECK(fty::Storage::clear());
}
