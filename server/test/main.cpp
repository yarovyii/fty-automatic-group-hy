#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_DISABLE_EXCEPTIONS

#include <catch2/catch.hpp>
#include "lib/config.h"
#include <fty_log.h>
#include <asset/test-db.h>
#include <asset/db.h>

int main(int argc, char* argv[])
{
    if (auto ret = fty::Config::instance().load("test/conf/agroup.conf")) {
        ManageFtyLog::setInstanceFtylog(fty::Config::instance().actorName, fty::Config::instance().logger);
        fty::TestDb db;
        if (auto res = db.create()) {
            std::string url = *res;
            setenv("DBURL", url.c_str(), 1);
            int result = Catch::Session().run(argc, argv);
            //tnt::threadEnd();
            db.destroy();
            return result;
        } else {
            std::cerr << res.error() << std::endl;
            return EXIT_FAILURE;
        }
    } else {
        std::cerr << ret.error() << std::endl;
        return EXIT_FAILURE;
    }
}
