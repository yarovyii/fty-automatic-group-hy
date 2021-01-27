#pragma once
#include <fty/expected.h>
#include <pack/pack.h>

namespace fty {

class Config : public pack::Node
{
public:
    pack::String dbpath    = FIELD("dbpath");
    pack::String logger    = FIELD("logger");
    pack::String actorName = FIELD("actor-name", "automatic-group");

    using pack::Node::Node;
    META(Config, dbpath, logger, actorName);

public:
    static Config& instance();
    Expected<void> load(const std::string& path = "agroup.conf");
    Expected<void> reload();
private:
    std::string m_path;
};

} // namespace fty
