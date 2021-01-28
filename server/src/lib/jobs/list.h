#pragma once
#include "lib/task.h"

namespace fty::job {

class List: public Task<List, void, commands::list::Out>
{
public:
    using Task::Task;
    void run(commands::list::Out& out);
};

}
