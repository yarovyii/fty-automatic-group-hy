#include "db.h"
#include "lib/jobs/create.h"
#include "lib/jobs/read.h"
#include "lib/storage.h"
#include <catch2/catch.hpp>


TEST_CASE("Read")
{
    REQUIRE(fty::GroupsDB::init());

    std::string json = R"(
        {
            "name": "By types",
            "rules": {
                "conditions": [
                    {
                        "field": "type",
                        "operator": "CONTAINS",
                        "value": "srv"
                    }
                ],
                "operator": "OR"
            }
        }
    )";
    fty::Group group;
    pack::json::deserialize(json, group);

    fty::job::Create create;
    fty::Group created;
    REQUIRE_NOTHROW(create.run(group, created));
    REQUIRE(created.check());

    fty::job::Read read;
    fty::commands::read::In in;
    fty::commands::read::Out out;

    in.id = created.id;
    REQUIRE_NOTHROW(read.run(in, out));
    REQUIRE(out.hasValue());

    CHECK(out.id == created.id);
    CHECK(out.name == created.name);
    CHECK(out.rules.groupOp == created.rules.groupOp);
    CHECK(out.rules.conditions.size() == created.rules.conditions.size());

    in.id = 0;
    REQUIRE_THROWS_WITH(read.run(in, out), "Not found");

    CHECK(fty::Storage::clear());
}
