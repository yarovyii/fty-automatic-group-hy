#include "config.h"

namespace fty {

Config& Config::instance()
{
    static Config conf;
    return conf;
}

Expected<void> Config::load(const std::string& path)
{
    if (auto ret = pack::yaml::deserializeFile(path, *this); !ret) {
        return unexpected(ret.error());
    }
    return {};
}

}
