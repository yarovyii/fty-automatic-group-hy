#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_DISABLE_EXCEPTIONS

#include <catch2/catch.hpp>
#include "lib/config.h"

int main(int argc, char* argv[])
{
    if (auto ret = fty::Config::instance().load("test/conf/agroup.conf")) {
        int result = Catch::Session().run(argc, argv);
        return result;
    } else {
        std::cerr << ret.error() << std::endl;
        return EXIT_FAILURE;
    }
}
