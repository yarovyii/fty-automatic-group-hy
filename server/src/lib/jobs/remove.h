#pragma once
#include "lib/task.h"

namespace fty::job {

class Remove: public Task<Remove, commands::remove::In, commands::remove::Out>
{
public:
    using Task::Task;
    void run(const commands::remove::In& in, commands::remove::Out& out);
};

}
