#include "config.h"

namespace fty {

Config& Config::instance()
{
    static Config conf;
    return conf;
}

Expected<void> Config::load(const std::string& path)
{
    m_path = path;
    return reload();
}

Expected<void> Config::reload()
{
    if (auto ret = pack::yaml::deserializeFile(m_path, *this); !ret) {
        return unexpected(ret.error());
    }
    return {};
}

}
