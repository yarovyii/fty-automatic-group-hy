#include <catch2/catch.hpp>
#include "lib/jobs/create.h"
#include "lib/jobs/remove.h"
#include "lib/jobs/resolve.h"
#include "db.h"
#include "lib/storage.h"

class Group: public fty::Group
{
public:
    Group() = default;

    Group(fty::Group&& other):
        fty::Group(other)
    {
    }

    ~Group() override
    {
        remove();
    }

    Group create()
    {
        fty::job::Create create;
        fty::commands::create::Out created;
        REQUIRE_NOTHROW(create.run(*this, created));
        return Group(std::move(created));
    }

    void remove()
    {
        if (id.hasValue()) {
            fty::job::Remove remove;

            fty::commands::remove::In in;
            fty::commands::remove::Out out;
            in.append(id);
            REQUIRE_NOTHROW(remove.run(in, out));
            REQUIRE(out.size() == 1);
            CHECK(out[0][fty::convert<std::string>(id.value())] == "Ok");
            clear();
        }
    }

    fty::commands::resolve::Out resolve()
    {
        fty::job::Resolve resolve;

        fty::commands::resolve::In in;
        fty::commands::resolve::Out out;

        in.id = id;
        REQUIRE_NOTHROW(resolve.run(in, out));

        return out;
    }
};

TEST_CASE("Resolve by name")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByName";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var  = group.rules.conditions.append();
    auto& cond = var.reset<fty::Group::Condition>();
    cond.field = fty::Group::Fields::Name;

    SECTION("Contains")
    {
        cond.value = "srv1";
        cond.op    = fty::Group::ConditionOp::Contains;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "srv1");
        CHECK(info[3].name == "srv11");
    }

    SECTION("DoesNotContain")
    {
        cond.value = "srv1";
        cond.op    = fty::Group::ConditionOp::DoesNotContain;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 3);
        CHECK(info[0].name == "srv2");
        CHECK(info[1].name == "srv3");
        CHECK(info[3].name == "srv21");
    }


    SECTION("Is")
    {
        cond.value = "srv1";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");
    }

    SECTION("IsNot")
    {
        cond.value = "srv1";
        cond.op    = fty::Group::ConditionOp::IsNot;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 4);
        CHECK(info[0].name == "srv2");
        CHECK(info[1].name == "srv3");
        CHECK(info[2].name == "srv11");
        CHECK(info[3].name == "srv21");
    }

    SECTION("Not exists")
    {
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 0);
    }

    CHECK(fty::Storage::clear());
}

TEST_CASE("Resolve by InternalName")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByInternalName";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var  = group.rules.conditions.append();
    auto& cond = var.reset<fty::Group::Condition>();
    cond.field = fty::Group::Fields::InternalName;

    SECTION("Contains")
    {
        cond.value = "srv";
        cond.op    = fty::Group::ConditionOp::Contains;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");
    }

    SECTION("Is")
    {
        cond.value = "srv1";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");
    }

    SECTION("IsNot")
    {
        cond.value = "srv1";
        cond.op    = fty::Group::ConditionOp::IsNot;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 6);
        CHECK(info[0].name == "datacenter");
        CHECK(info[1].name == "datacenter1");
        CHECK(info[2].name == "srv2");
        CHECK(info[3].name == "srv3");
        CHECK(info[4].name == "srv11");
        CHECK(info[5].name == "srv21");
    }

    SECTION("Not exists")
    {
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 0);
    }

    CHECK(fty::Storage::clear());
}

TEST_CASE("Resolve by location")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByLocation";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var  = group.rules.conditions.append();
    auto& cond = var.reset<fty::Group::Condition>();
    cond.field = fty::Group::Fields::Location;

    SECTION("Contains")
    {
        cond.value = "datacenter";
        cond.op    = fty::Group::ConditionOp::Contains;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");
    }

    SECTION("Is")
    {
        cond.value = "datacenter";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 3);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
    }

    SECTION("IsNot")
    {
        cond.value = "datacenter";
        cond.op    = fty::Group::ConditionOp::IsNot;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "srv11");
        CHECK(info[1].name == "srv21");
    }

    SECTION("Not exists")
    {
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 0);
    }


    CHECK(fty::Storage::clear());
}

TEST_CASE("Resolve by type")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByType";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var  = group.rules.conditions.append();
    auto& cond = var.reset<fty::Group::Condition>();
    cond.field = fty::Group::Fields::Type;

    SECTION("Contains")
    {
        cond.value = "device";
        cond.op    = fty::Group::ConditionOp::Contains;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");
    }

    SECTION("Is")
    {
        cond.value = "device";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");
    }

    SECTION("IsNot")
    {
        cond.value = "device";
        cond.op    = fty::Group::ConditionOp::IsNot;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "datacenter");
        CHECK(info[1].name == "datacenter1");
    }

    SECTION("Not exists")
    {
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g= group.create();
        auto info = g.resolve();
        REQUIRE(info.size() == 0);
    }

    CHECK(fty::Storage::clear());
}


TEST_CASE("Resolve by subtype")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "BySubType";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var  = group.rules.conditions.append();
    auto& cond = var.reset<fty::Group::Condition>();
    cond.field = fty::Group::Fields::SubType;

    SECTION("Contains")
    {
        cond.value = "server";
        cond.op    = fty::Group::ConditionOp::Contains;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");
    }

    SECTION("Is")
    {
        cond.value = "server";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");
    }

    SECTION("IsNot")
    {
        cond.value = "server";
        cond.op    = fty::Group::ConditionOp::IsNot;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "datacenter");
        CHECK(info[1].name == "datacenter1");
    }

    SECTION("Not exists")
    {
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g= group.create();
        auto info = g.resolve();
        REQUIRE(info.size() == 0);
    }

    CHECK(fty::Storage::clear());
}

TEST_CASE("Resolve by hostname")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByHostName";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var  = group.rules.conditions.append();
    auto& cond = var.reset<fty::Group::Condition>();
    cond.field = fty::Group::Fields::HostName;

    SECTION("Contains")
    {
        cond.value = "local";
        cond.op    = fty::Group::ConditionOp::Contains;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv11");
    }

    SECTION("Is")
    {
        cond.value = "localhost";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv11");
    }

    SECTION("IsNot")
    {
        cond.value = "localhost";
        cond.op    = fty::Group::ConditionOp::IsNot;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 4);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv21");
    }

    SECTION("Not exists")
    {
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 0);
    }

    CHECK(fty::Storage::clear());
}

TEST_CASE("Resolve by contact")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByContact";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var  = group.rules.conditions.append();
    auto& cond = var.reset<fty::Group::Condition>();
    cond.field = fty::Group::Fields::Contact;

    SECTION("Contains")
    {
        cond.value = "dim";
        cond.op    = fty::Group::ConditionOp::Contains;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "srv11");
        CHECK(info[1].name == "srv21");
    }

    SECTION("Is")
    {
        cond.value = "dim";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv11");
    }

    SECTION("Is not")
    {
        cond.value = "dim";
        cond.op    = fty::Group::ConditionOp::IsNot;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 6);
        CHECK(info[0].name == "datacenter");
        CHECK(info[1].name == "datacenter1");
        CHECK(info[2].name == "srv1");
        CHECK(info[3].name == "srv2");
        CHECK(info[4].name == "srv3");
        CHECK(info[5].name == "srv21");
    }

    SECTION("Not exists")
    {
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 0);
    }

    CHECK(fty::Storage::clear());
}

TEST_CASE("Resolve by ip address")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByIpAddress";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var  = group.rules.conditions.append();
    auto& cond = var.reset<fty::Group::Condition>();
    cond.field = fty::Group::Fields::IPAddress;

    SECTION("Contains")
    {
        cond.value = "127.0";
        cond.op    = fty::Group::ConditionOp::Contains;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");
    }

    SECTION("Is")
    {
        cond.value = "127.0.0.1";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");
    }

    SECTION("Is/mask")
    {
        cond.value = "127.0.*";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");
    }

    SECTION("Is/few")
    {
        cond.value = "127.0.*|192.168.*";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
    }

    SECTION("Is not")
    {
        cond.value = "127.0.0.1";
        cond.op    = fty::Group::ConditionOp::IsNot;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 4);
        CHECK(info[0].name == "srv2");
        CHECK(info[1].name == "srv3");
        CHECK(info[2].name == "srv11");
        CHECK(info[3].name == "srv21");
    }

    SECTION("Not exists")
    {
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 0);
    }

    CHECK(fty::Storage::clear());
}

TEST_CASE("Resolve by name and location")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByName";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    {
        auto& var1  = group.rules.conditions.append();
        auto& cond1 = var1.reset<fty::Group::Condition>();
        cond1.field = fty::Group::Fields::Name;

        auto& var2  = group.rules.conditions.append();
        auto& cond2 = var2.reset<fty::Group::Condition>();
        cond2.field = fty::Group::Fields::Location;
    }

    SECTION("Contains")
    {
        auto& cond1 = group.rules.conditions[0].get<fty::Group::Condition>();
        cond1.value = "srv";
        cond1.op    = fty::Group::ConditionOp::Contains;

        auto& cond2 = group.rules.conditions[1].get<fty::Group::Condition>();
        cond2.value = "datacenter";
        cond2.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 3);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
    }

    SECTION("Is")
    {
        auto& cond1 = group.rules.conditions[0].get<fty::Group::Condition>();
        cond1.value = "srv21";
        cond1.op    = fty::Group::ConditionOp::Is;

        auto& cond2 = group.rules.conditions[1].get<fty::Group::Condition>();
        cond2.value = "datacenter1";
        cond2.op    = fty::Group::ConditionOp::Is;

        auto g = group.create();
        auto info = g.resolve();

        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv21");
    }

    CHECK(fty::Storage::clear());
}

TEST_CASE("Resolve by name and location/mutlty")
{
    REQUIRE(fty::GroupsDB::init());

    Group group;
    group.name          = "ByName";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    auto& var1  = group.rules.conditions.append();
    auto& cond1 = var1.reset<fty::Group::Condition>();
    cond1.op = fty::Group::ConditionOp::Is;
    cond1.value = "datacenter1";
    cond1.field = fty::Group::Fields::Location;

    auto& var2  = group.rules.conditions.append();
    auto& rules = var2.reset<fty::Group::Rules>();
    {
        rules.groupOp = fty::Group::LogicalOp::Or;

        auto& scond1 = rules.conditions.append().reset<fty::Group::Condition>();
        scond1.op = fty::Group::ConditionOp::Is;
        scond1.field = fty::Group::Fields::Name;
        scond1.value = "srv21";

        auto& scond2 = rules.conditions.append().reset<fty::Group::Condition>();
        scond2.op = fty::Group::ConditionOp::Is;
        scond2.field = fty::Group::Fields::Name;
        scond2.value = "srv11";
    }

    auto g = group.create();
    auto info = g.resolve();

    REQUIRE(info.size() == 2);
    CHECK(info[0].name == "srv11");
    CHECK(info[1].name == "srv21");
}

