#pragma once
#include <pack/pack.h>
#include <fty/expected.h>

namespace fty {

class Config: public pack::Node
{
public:
    pack::String dbpath = FIELD("dbpath");
    pack::String logger = FIELD("logger");

    using pack::Node::Node;
    META(Config, dbpath);

public:
    static Config& instance();
    Expected<void> load(const std::string& path = "agroup.conf");
};

}

