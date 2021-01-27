#include "common/commands.h"
#include "common/message-bus.h"
#include "lib/config.h"
#include "lib/server.h"
#include <catch2/catch.hpp>

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

    fty::Message msg;
    msg.meta.to      = fty::Config::instance().actorName;
    msg.meta.subject = "CREATE";
    msg.meta.from    = "unit-test";
    msg.userData.setString("Some shit");

    auto ret = bus.send(fty::Channel, msg);
    CHECK(ret);
}
