#define CATCH_CONFIG_RUNNER

#include "lib/config.h"
#include <catch2/catch.hpp>
#include <fty_log.h>
#include <filesystem>
#include "db.h"

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
        int result = session.run(argc, argv);
        fty::GroupsDB::destroy();
        return result;
    } else {
        std::cerr << ret.error() << std::endl;
        return EXIT_FAILURE;
    }
}
