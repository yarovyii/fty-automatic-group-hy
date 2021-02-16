#include "common/commands.h"
#include "common/message-bus.h"
#include "lib/config.h"
#include "lib/server.h"
#include "test-utils.h"
#include <catch2/catch.hpp>
#include <asset/db.h>
#include <asset/test-db.h>

static fty::Message message(const std::string& subj)
{
    fty::Message msg;
    msg.meta.to      = fty::Config::instance().actorName;
    msg.meta.subject = subj;
    msg.meta.from    = "unit-test";
    return msg;
}

struct Listener
{
    void event(const fty::Message& msg)
    {
        std::cerr << msg.meta.subject.value() << std::endl;
    }
};

TEST_CASE("Server request")
{
    fty::TestDb db;
    if (auto res = db.create()) {
        std::string url = *res;
        setenv("DBURL", url.c_str(), 1);
    } else {
        FAIL(res.error());
    }

    fty::Server srv;
    auto        res = srv.run();
    if (!res) {
        FAIL(res.error());
    }
    REQUIRE(res);

    fty::MessageBus bus;
    if (auto ret = bus.init("unit-test"); !ret) {
        FAIL(ret.error());
    }

    Listener listen;
    if (auto sub = bus.subsribe(fty::Events, &Listener::event, &listen); !sub) {
        FAIL(sub.error());
    }


    // Db
    assets::DataCenter dc("datacenter");
    assets::Server     srv1("srv1", dc);
    srv1.setExtName("srv1");
    assets::Server srv2("srv2", dc);
    srv2.setExtName("srv2");
    assets::Server srv3("srv3", dc);
    srv3.setExtName("srv3");

    assets::DataCenter dc1("datacenter1");
    assets::Server     srv11("srv11", dc1);
    srv11.setExtName("srv11");
    assets::Server srv21("srv21", dc1);
    srv21.setExtName("srv21");

    // Group
    fty::Group group;
    group.name          = "My group";
    group.rules.groupOp = fty::Group::LogicalOp::And;

    {
        auto& var = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "srv";
        cond.field = fty::Group::Fields::Name;
        cond.op    = fty::Group::ConditionOp::Contains;
    }

    {
        auto& var = group.rules.conditions.append();
        auto& cond = var.reset<fty::Group::Condition>();
        cond.value = "datacenter";
        cond.field = fty::Group::Fields::Location;
        cond.op    = fty::Group::ConditionOp::Is;
    }

    // Create group
    {
        fty::Message msg = message(fty::commands::create::Subject);
        msg.userData.setString(*pack::json::serialize(group));
        auto ret = bus.send(fty::Channel, msg);
        REQUIRE(ret);
        CHECK(ret->meta.status == fty::Message::Status::Ok);
    }

    uint64_t gId = 0;
    // Read group's list
    {
        fty::Message msg = message(fty::commands::list::Subject);
        auto         ret = bus.send(fty::Channel, msg);
        REQUIRE(ret);

        auto info = ret->userData.decode<fty::commands::list::Out>();
        REQUIRE(info);
        CHECK(!info->empty());
        CHECK((*info)[0].name == "My group");
        gId = (*info)[0].id;
    }

    REQUIRE(gId > 0);

    // Read group
    {
        fty::Message msg = message(fty::commands::read::Subject);

        fty::commands::read::In in;
        in.id = gId;

        msg.userData.setString(*pack::json::serialize(in));

        auto ret = bus.send(fty::Channel, msg);
        REQUIRE(ret);

        auto info = ret->userData.decode<fty::commands::read::Out>();
        REQUIRE(info);
        CHECK((*info).name == "My group");
    }

    // resolve group
    {
        fty::Message msg = message(fty::commands::resolve::Subject);

        fty::commands::resolve::In in;
        in.id = gId;

        msg.userData.setString(*pack::json::serialize(in));
        auto ret = bus.send(fty::Channel, msg);
        REQUIRE(ret);
        auto info = ret->userData.decode<fty::commands::resolve::Out>();
        REQUIRE(info->size() == 3);
        CHECK((*info)[0].name == "srv1");
        CHECK((*info)[1].name == "srv2");
        CHECK((*info)[2].name == "srv3");
    }

    // Delete group
    {
        fty::Message msg = message(fty::commands::remove::Subject);

        fty::commands::remove::In in;
        in.append(gId);

        msg.userData.setString(*pack::json::serialize(in));
        auto ret = bus.send(fty::Channel, msg);
        REQUIRE(ret);
    }

    deleteAsset(srv1);
    deleteAsset(srv2);
    deleteAsset(srv3);
    deleteAsset(srv11);
    deleteAsset(srv21);
    deleteAsset(dc);
    deleteAsset(dc1);

    db.destroy();
}
