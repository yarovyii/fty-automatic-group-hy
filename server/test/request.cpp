#include "common/commands.h"
#include "common/message-bus.h"
#include "lib/config.h"
#include "lib/server.h"
#include "test-utils.h"
#include "common/logger.h"
#include <asset/db.h>
#include <asset/test-db.h>
#include <catch2/catch.hpp>


// =====================================================================================================================

struct Listener
{
    void event(const fty::Message& msg)
    {
        logDebug("Event: {}", msg.meta.subject.value());
    }
};

// =====================================================================================================================

class Group : public fty::Group
{
public:
    using fty::Group::Group;

    void create(fty::MessageBus& bus)
    {
        fty::Message msg = message(fty::commands::create::Subject);
        msg.userData.setString(*pack::json::serialize(*this));

        auto ret = bus.send(fty::Channel, msg);
        if (!ret) {
            FAIL(ret.error());
        }
        CHECK(ret->meta.status == fty::Message::Status::Ok);

        auto info = ret->userData.decode<fty::commands::create::Out>();
        if (!info) {
            FAIL(info.error());
        }

        REQUIRE(info->id > 0);
        id = info->id;
    }

    void update(fty::MessageBus& bus)
    {
        fty::Message msg = message(fty::commands::update::Subject);
        msg.userData.setString(*pack::json::serialize(*this));

        auto ret = bus.send(fty::Channel, msg);
        if (!ret) {
            FAIL(ret.error());
        }
        CHECK(ret->meta.status == fty::Message::Status::Ok);

        auto info = ret->userData.decode<fty::commands::update::Out>();
        if (!info) {
            FAIL(info.error());
        }
    }

    fty::commands::resolve::Out resolve(fty::MessageBus& bus)
    {
        fty::Message msg = message(fty::commands::resolve::Subject);

        fty::commands::resolve::In in;
        in.id = id;

        msg.userData.setString(*pack::json::serialize(in));
        auto ret = bus.send(fty::Channel, msg);
        if (!ret) {
            FAIL(ret.error());
        }

        auto info = ret->userData.decode<fty::commands::resolve::Out>();
        if (!info) {
            FAIL(info.error());
        }
        return *info;
    }

    void remove(fty::MessageBus& bus)
    {
        fty::Message msg = message(fty::commands::remove::Subject);

        fty::commands::remove::In in;
        in.append(id);

        msg.userData.setString(*pack::json::serialize(in));
        auto ret = bus.send(fty::Channel, msg);
        if (!ret) {
            FAIL(ret.error());
        }
    }

    static fty::commands::list::Out list(fty::MessageBus& bus)
    {
        fty::Message msg = message(fty::commands::list::Subject);
        auto         ret = bus.send(fty::Channel, msg);
        if (!ret) {
            FAIL(ret.error());
        }

        auto info = ret->userData.decode<fty::commands::list::Out>();
        if (!info) {
            FAIL(info.error());
        }
        return *info;
    }

    fty::commands::read::Out read(fty::MessageBus& bus)
    {
        fty::Message msg = message(fty::commands::read::Subject);

        fty::commands::read::In in;
        in.id = id;

        msg.userData.setString(*pack::json::serialize(in));

        auto ret = bus.send(fty::Channel, msg);
        if (!ret) {
            FAIL(ret.error());
        }

        auto info = ret->userData.decode<fty::commands::read::Out>();
        if (!info) {
            FAIL(info.error());
        }
        return *info;
    }

private:
    static fty::Message message(const std::string& subj)
    {
        fty::Message msg;
        msg.meta.to      = fty::Config::instance().actorName;
        msg.meta.subject = subj;
        msg.meta.from    = "unit-test";
        return msg;
    }
};

// =====================================================================================================================

struct Test
{
    void init()
    {
        if (auto res = db.create()) {
            std::string url = *res;
            setenv("DBURL", url.c_str(), 1);
        } else {
            FAIL(res.error());
        }

        auto res = srv.run();
        if (!res) {
            FAIL(res.error());
        }
        REQUIRE(res);

        if (auto ret = bus.init("unit-test"); !ret) {
            FAIL(ret.error());
        }

        if (auto sub = bus.subsribe(fty::Events, &Listener::event, &listen); !sub) {
            FAIL(sub.error());
        }


        // Db
        dc   = std::make_unique<assets::DataCenter>("datacenter");
        srv1 = std::make_unique<assets::Server>("srv1", *dc, "srv1");
        srv2 = std::make_unique<assets::Server>("srv2", *dc, "srv2");
        srv3 = std::make_unique<assets::Server>("srv3", *dc, "srv3");

        dc1   = std::make_unique<assets::DataCenter>("datacenter1");
        srv11 = std::make_unique<assets::Server>("srv11", *dc1, "srv11");
        srv21 = std::make_unique<assets::Server>("srv21", *dc1, "srv21");

        srv11->setExtAttributes({{"device.contact", "dim"}, {"hostname.1", "localhost"}});
        srv21->setExtAttributes({{"contact_email", "dim@eaton.com"}});
        srv1->setExtAttributes({{"ip.1", "127.0.0.1"}});
        srv2->setExtAttributes({{"ip.1", "192.168.0.1"}});
    }

    ~Test()
    {
        srv.shutdown();
        deleteAsset(*srv1);
        deleteAsset(*srv2);
        deleteAsset(*srv3);
        deleteAsset(*srv11);
        deleteAsset(*srv21);
        deleteAsset(*dc);
        deleteAsset(*dc1);

        db.destroy();
    }

    fty::TestDb     db;
    fty::MessageBus bus;
    Listener        listen;
    fty::Server     srv;

    std::unique_ptr<assets::DataCenter> dc;
    std::unique_ptr<assets::DataCenter> dc1;
    std::unique_ptr<assets::Server>     srv1;
    std::unique_ptr<assets::Server>     srv2;
    std::unique_ptr<assets::Server>     srv3;
    std::unique_ptr<assets::Server>     srv11;
    std::unique_ptr<assets::Server>     srv21;
};

// =====================================================================================================================

static void testSimpleGroup(fty::MessageBus& bus)
{
    // Group
    Group group;
    group.name          = "My group";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    {
        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "srv";
        cond.field = fty::Group::Fields::Name;
        cond.op    = fty::Group::ConditionOp::Contains;
    }

    {
        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "datacenter";
        cond.field = fty::Group::Fields::Location;
        cond.op    = fty::Group::ConditionOp::Is;
    }

    // Create group
    group.create(bus);

    // Read group's list
    auto lst = Group::list(bus);
    CHECK(!lst.empty());
    CHECK(lst[0].name == "My group");


    // Read group
    auto info = group.read(bus);
    CHECK(info.name == "My group");
    CHECK(info.id == group.id);
    CHECK(info.rules.groupOp == fty::Group::LogicalOp::And);
    CHECK(info.rules.conditions.size() == 2);
    CHECK(info.rules.conditions[0].is<fty::Group::Condition>());
    CHECK(info.rules.conditions[1].is<fty::Group::Condition>());
    {
        const auto& cond = info.rules.conditions[0].get<fty::Group::Condition>();
        CHECK(cond.field == fty::Group::Fields::Name);
        CHECK(cond.op == fty::Group::ConditionOp::Contains);
        CHECK(cond.value == "srv");
    }
    {
        const auto& cond = info.rules.conditions[1].get<fty::Group::Condition>();
        CHECK(cond.field == fty::Group::Fields::Location);
        CHECK(cond.op == fty::Group::ConditionOp::Is);
        CHECK(cond.value == "datacenter");
    }

    // resolve group
    auto res = group.resolve(bus);
    REQUIRE(res.size() == 3);
    CHECK(res[0].name == "srv1");
    CHECK(res[1].name == "srv2");
    CHECK(res[2].name == "srv3");

    // Delete group
    group.remove(bus);
}

// =====================================================================================================================

static void testByName(fty::MessageBus& bus)
{
    // Contains operator
    {
        Group group;
        group.name          = "ByName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "srv";
        cond.field = fty::Group::Fields::Name;
        cond.op    = fty::Group::ConditionOp::Contains;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");

        group.remove(bus);
    }
    // Is operator
    {
        Group group;
        group.name          = "ByName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "srv1";
        cond.field = fty::Group::Fields::Name;
        cond.op    = fty::Group::ConditionOp::Is;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");

        group.remove(bus);
    }
    // IsNot operator
    {
        Group group;
        group.name          = "ByName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "srv1";
        cond.field = fty::Group::Fields::Name;
        cond.op    = fty::Group::ConditionOp::IsNot;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 4);
        CHECK(info[0].name == "srv2");
        CHECK(info[1].name == "srv3");
        CHECK(info[2].name == "srv11");
        CHECK(info[3].name == "srv21");

        group.remove(bus);
    }
    // Not exists
    {
        Group group;
        group.name          = "ByName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "wtf";
        cond.field = fty::Group::Fields::Name;
        cond.op    = fty::Group::ConditionOp::Is;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 0);

        group.remove(bus);
    }
}

// =====================================================================================================================

static void testByLocation(fty::MessageBus& bus)
{
    // Contains operator
    {
        Group group;
        group.name          = "ByLocation";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "datacenter";
        cond.field = fty::Group::Fields::Location;
        cond.op    = fty::Group::ConditionOp::Contains;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");

        group.remove(bus);
    }
    // Is operator
    {
        Group group;
        group.name          = "ByLocation";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "datacenter";
        cond.field = fty::Group::Fields::Location;
        cond.op    = fty::Group::ConditionOp::Is;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 3);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");

        group.remove(bus);
    }
    // IsNot operator
    {
        Group group;
        group.name          = "ByLocation";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "datacenter";
        cond.field = fty::Group::Fields::Location;
        cond.op    = fty::Group::ConditionOp::IsNot;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "srv11");
        CHECK(info[1].name == "srv21");

        group.remove(bus);
    }
    // Not exists
    {
        Group group;
        group.name          = "ByLocation";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "wtf";
        cond.field = fty::Group::Fields::Location;
        cond.op    = fty::Group::ConditionOp::Is;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 0);

        group.remove(bus);
    }
}

static void testByType(fty::MessageBus& bus)
{
    // Contains operator
    {
        Group group;
        group.name          = "ByType";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "server";
        cond.field = fty::Group::Fields::Type;
        cond.op    = fty::Group::ConditionOp::Contains;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");

        group.remove(bus);
    }
    // Is operator
    {
        Group group;
        group.name          = "ByType";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "server";
        cond.field = fty::Group::Fields::Type;
        cond.op    = fty::Group::ConditionOp::Is;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 5);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv11");
        CHECK(info[4].name == "srv21");

        group.remove(bus);
    }
    // IsNot operator
    {
        Group group;
        group.name          = "ByType";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "server";
        cond.field = fty::Group::Fields::Type;
        cond.op    = fty::Group::ConditionOp::IsNot;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "datacenter");
        CHECK(info[1].name == "datacenter1");

        group.remove(bus);
    }
    // Not exists
    {
        Group group;
        group.name          = "ByType";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "wtf";
        cond.field = fty::Group::Fields::Type;
        cond.op    = fty::Group::ConditionOp::Is;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 0);

        group.remove(bus);
    }
}

static void testByHostName(fty::MessageBus& bus)
{
    // Contains operator
    {
        Group group;
        group.name          = "ByHostName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "local";
        cond.field = fty::Group::Fields::HostName;
        cond.op    = fty::Group::ConditionOp::Contains;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv11");

        group.remove(bus);
    }
    // Is operator
    {
        Group group;
        group.name          = "ByHostName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "localhost";
        cond.field = fty::Group::Fields::HostName;
        cond.op    = fty::Group::ConditionOp::Is;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv11");

        group.remove(bus);
    }
    // IsNot operator
    {
        Group group;
        group.name          = "ByHostName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "localhost";
        cond.field = fty::Group::Fields::HostName;
        cond.op    = fty::Group::ConditionOp::IsNot;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 4);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
        CHECK(info[2].name == "srv3");
        CHECK(info[3].name == "srv21");

        group.remove(bus);
    }
    // Not exists
    {
        Group group;
        group.name          = "ByHostName";
        group.rules.groupOp = fty::Group::LogicalOp::And;

        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "wtf";
        cond.field = fty::Group::Fields::Type;
        cond.op    = fty::Group::ConditionOp::Is;

        group.create(bus);
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 0);

        group.remove(bus);
    }
}

static void testByContact(fty::MessageBus& bus)
{
    Group group;
    group.name          = "ByContact";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    {
        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "dim";
        cond.field = fty::Group::Fields::Contact;
        cond.op    = fty::Group::ConditionOp::Contains;
    }
    group.create(bus);

    // Contains operator
    {
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "srv11");
        CHECK(info[1].name == "srv21");
    }
    // Is operator
    {
        auto& cond = group.rules.conditions[0].get<fty::Group::Condition>();
        cond.value = "dim";
        cond.op    = fty::Group::ConditionOp::Is;
        group.update(bus);

        auto info = group.resolve(bus);
        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv11");
    }
    // IsNot operator
    {
        auto& cond = group.rules.conditions[0].get<fty::Group::Condition>();
        cond.value = "dim";
        cond.op    = fty::Group::ConditionOp::IsNot;
        group.update(bus);

        auto info = group.resolve(bus);
        REQUIRE(info.size() == 6);
        CHECK(info[0].name == "datacenter");
        CHECK(info[1].name == "srv1");
        CHECK(info[2].name == "srv2");
        CHECK(info[3].name == "srv3");
        CHECK(info[4].name == "datacenter1");
        CHECK(info[5].name == "srv21");
    }
    // Not exists
    {
        auto& cond = group.rules.conditions[0].get<fty::Group::Condition>();
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;
        group.update(bus);

        auto info = group.resolve(bus);
        REQUIRE(info.size() == 0);
    }
    group.remove(bus);
}

static void testByIpAddress(fty::MessageBus& bus)
{
    Group group;
    group.name          = "IpAddress";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    {
        auto& var  = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "127.0";
        cond.field = fty::Group::Fields::IPAddress;
        cond.op    = fty::Group::ConditionOp::Contains;
    }
    group.create(bus);

    // Contains operator
    {
        auto info = group.resolve(bus);
        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");
    }
    // Is operator
    {
        auto& cond = group.rules.conditions[0].get<fty::Group::Condition>();
        cond.value = "127.0.0.1";
        cond.op    = fty::Group::ConditionOp::Is;
        group.update(bus);

        auto info = group.resolve(bus);
        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");
    }
    // Is operator
    {
        auto& cond = group.rules.conditions[0].get<fty::Group::Condition>();
        cond.value = "127.0.*";
        cond.op    = fty::Group::ConditionOp::Is;
        group.update(bus);

        auto info = group.resolve(bus);
        REQUIRE(info.size() == 1);
        CHECK(info[0].name == "srv1");
    }
    // Is operator
    {
        auto& cond = group.rules.conditions[0].get<fty::Group::Condition>();
        cond.value = "127.0.*|192.168.*";
        cond.op    = fty::Group::ConditionOp::Is;
        group.update(bus);

        auto info = group.resolve(bus);
        REQUIRE(info.size() == 2);
        CHECK(info[0].name == "srv1");
        CHECK(info[1].name == "srv2");
    }
    // IsNot operator
    {
        auto& cond = group.rules.conditions[0].get<fty::Group::Condition>();
        cond.value = "127.0.0.1";
        cond.op    = fty::Group::ConditionOp::IsNot;
        group.update(bus);

        auto info = group.resolve(bus);
        REQUIRE(info.size() == 4);
        CHECK(info[0].name == "srv2");
        CHECK(info[1].name == "srv3");
        CHECK(info[2].name == "srv11");
        CHECK(info[3].name == "srv21");
    }
    // Not exists
    {
        auto& cond = group.rules.conditions[0].get<fty::Group::Condition>();
        cond.value = "wtf";
        cond.op    = fty::Group::ConditionOp::Is;
        group.update(bus);

        auto info = group.resolve(bus);
        REQUIRE(info.size() == 0);
    }
    group.remove(bus);
}

// =====================================================================================================================

TEST_CASE("Server request")
{
    std::unique_ptr<Test> test;
    if (!test) {
        test = std::make_unique<Test>();
        test->init();
    }

    testSimpleGroup(test->bus);
    testByName(test->bus);
    testByLocation(test->bus);
    testByType(test->bus);
    testByHostName(test->bus);
    testByContact(test->bus);
    testByIpAddress(test->bus);
}
