#pragma once
#include "lib/task.h"

namespace fty::job {

class Read: public Task<Read, commands::read::In, commands::read::Out>
{
public:
    using Task::Task;
    void run(const commands::read::In& cmd, commands::read::Out& out);
};

}
