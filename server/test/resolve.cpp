#include <catch2/catch.hpp>
#include "test-utils.h"

TEST_CASE("Resolve")
{
    assets::DataCenter dc("datacenter");

    deleteAsset(dc);
}
