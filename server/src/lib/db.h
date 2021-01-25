#pragma once
#include "group.h"
#include <fty/expected.h>

namespace fty {

class Storage
{
public:
    static Storage& instance();

public:
    Storage();
    ~Storage();

    static Expected<Group>          byName(const std::string& name);
    static Expected<Group>          byId(uint64_t id);
    static std::vector<std::string> names();
    static std::vector<uint64_t>    ids();

    static Expected<Group> save(const Group& group);
    static Expected<void>  remove(uint64_t id);
    static Expected<void>  removeByName(const std::string& groupName);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace fty
