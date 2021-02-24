#include "common/logger.h"
#include "lib/config.h"
#include "lib/daemon.h"
#include "lib/server.h"
#include <fty/command-line.h>
#include <iostream>

int main(int argc, char** argv)
{
    bool        daemon = false;
    std::string config = "conf/agroup.conf";
    bool        help   = false;

    // clang-format off
    fty::CommandLine cmd("Automatic group service", {
        {"--config", config, "Configuration file"},
        {"--daemon", daemon, "Daemonize this application"},
        {"--help",   help,   "Show this help"}
    });
    // clang-format on

    if (auto res = cmd.parse(argc, argv); !res) {
        std::cerr << res.error() << std::endl;
        std::cout << std::endl;
        std::cout << cmd.help() << std::endl;
        return EXIT_FAILURE;
    }

    if (help) {
        std::cout << cmd.help() << std::endl;
        return EXIT_SUCCESS;
    }

    if (auto ret = fty::Config::instance().load(config); !ret) {
        logError(ret.error());
        return EXIT_FAILURE;
    }

    ManageFtyLog::setInstanceFtylog(fty::Config::instance().actorName, fty::Config::instance().logger);

    if (daemon) {
        logDebug("Start automatic group agent as daemon");
        fty::Daemon::daemonize();
    }

    fty::Server srv;
    if (auto res = srv.run()) {
        srv.wait();
        srv.shutdown();
    } else {
        logError(res.error());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
