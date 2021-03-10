#pragma once

#include "common/group.h"
#include <fty/expected.h>
#include <list>
#include <map>
#include <pack/pack.h>
#include <string>

namespace fty::job::srr::utils {

// job wrappers
fty::Expected<std::list<uint64_t>> list();
fty::Expected<Group>               read(const uint64_t id);
fty::Expected<Group>               create(const Group& group);
std::map<std::string, std::string> remove(const std::list<uint64_t> idList);
std::map<std::string, std::string> clear();

} // namespace fty::job::srr::utils
