#pragma once
#include "lib/task.h"

namespace fty::job {

class Resolve: public Task<Resolve, commands::resolve::In, commands::resolve::Out>
{
public:
    using Task::Task;
    void run(const commands::resolve::In& groupId, commands::resolve::Out& assetList);
};

}

