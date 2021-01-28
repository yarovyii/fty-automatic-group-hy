#include "common/commands.h"
#include "common/message-bus.h"
#include "lib/config.h"
#include "lib/server.h"
#include <catch2/catch.hpp>

static fty::Message message(const std::string& subj)
{
    fty::Message msg;
    msg.meta.to      = fty::Config::instance().actorName;
    msg.meta.subject = subj;
    msg.meta.from    = "unit-test";
    return msg;
}

TEST_CASE("Server request")
{
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

    // Group
    fty::Group group;
    group.name     = "My group";
    group.rules.op = fty::Group::LogicalOp::And;

    auto& cond  = group.rules.conditions.append();
    cond.entity = "feed";
    cond.field  = "name";
    cond.op     = fty::Group::ConditionOp::Contains;

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
        auto ret = bus.send(fty::Channel, msg);
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
        msg.userData.setString(fty::convert<std::string>(gId));
        auto ret = bus.send(fty::Channel, msg);
        REQUIRE(ret);

        auto info = ret->userData.decode<fty::commands::read::Out>();
        REQUIRE(info);
        CHECK((*info).name == "My group");
    }

    // Delete group
    {
        fty::Message msg = message(fty::commands::remove::Subject);
        msg.userData.setString(fty::convert<std::string>(gId));
        auto ret = bus.send(fty::Channel, msg);
        REQUIRE(ret);
    }
}
