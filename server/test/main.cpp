#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_DISABLE_EXCEPTIONS

#include "lib/config.h"
#include <asset/db.h>
#include <asset/test-db.h>
#include <catch2/catch.hpp>
#include <fty_log.h>

int main(int argc, char* argv[])
{
    Catch::Session session;

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) {
        return returnCode;
    }

    Catch::ConfigData data = session.configData();
    if (data.listReporters || data.listTestNamesOnly) {
        return session.run();
    }

    if (auto ret = fty::Config::instance().load("test/conf/agroup.conf")) {
        ManageFtyLog::setInstanceFtylog(fty::Config::instance().actorName, fty::Config::instance().logger);
        std::filesystem::remove(fty::Config::instance().dbpath.value());
        fty::TestDb db;
        if (auto res = db.create()) {
            std::string url = *res;
            setenv("DBURL", url.c_str(), 1);
            int result = session.run(argc, argv);
            // tnt::threadEnd();
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
