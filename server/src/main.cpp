#include <fty/command-line.h>
#include <iostream>

int main(int argc, char** argv)
{
    bool        daemon = false;
    std::string config = "conf/discovery.conf";
    bool        help   = false;

    // clang-format off
    fty::CommandLine cmd("New discovery service", {
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

    return EXIT_SUCCESS;
}
