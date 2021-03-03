#pragma once
#include "common/message.h"
#include "lib/task.h"

namespace fty::job {

class SrrProcess: public Task<SrrProcess, void>
{
public:
    using Task::Task;
    void run();
};

}
