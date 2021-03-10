#include "utils.h"
#include "common/srr.h"
#include "lib/jobs/create.h"
#include "lib/jobs/list.h"
#include "lib/jobs/read.h"
#include "lib/jobs/remove.h"

namespace fty::job::srr::utils {

// list ids of all groups
Expected<std::list<uint64_t>> list()
{
    commands::list::Out data;
    job::List           job;

    try {
        job.run(data);
    } catch (const std::exception& e) {
        return unexpected("List job failed {}", e.what());
    }

    std::list<uint64_t> listId;

    for (const auto& x : data) {
        listId.push_back(x.id);
    }

    return listId;
}

// get info of group <id>
Expected<Group> read(const uint64_t id)
{
    commands::read::In  inData;
    commands::read::Out outData;
    job::Read           job;

    inData.id = id;

    try {
        job.run(inData, outData);
    } catch (const std::exception& e) {
        return unexpected("Read job failed {}", e.what());
    }

    return outData;
}

// create new group
Expected<Group> create(const Group& group)
{
    Group created;

    job::Create job;

    try {
        job.run(group, created);
    } catch (const std::exception& e) {
        return unexpected("Create job failed {}", e.what());
    }

    return created;
}

// remove list of groups
std::map<std::string, std::string> remove(const std::list<uint64_t> idList)
{
    std::map<std::string, std::string> retMap;
    job::Remove                        job;

    commands::remove::In  dataIn;
    commands::remove::Out dataOut;

    for (const auto id : idList) {
        dataIn.append(id);
    }

    job.run(dataIn, dataOut);

    for (const auto& el : dataOut) {
        retMap[el.key()] = el.value().at(el.key());
    }

    return retMap;
}

// remove all groups
std::map<std::string, std::string> clear()
{
    return remove(*list());
}

} // namespace fty::job::srr::utils
