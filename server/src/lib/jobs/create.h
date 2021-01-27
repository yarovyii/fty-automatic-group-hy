#pragma once
#include "lib/task.h"

namespace fty::job {

class Create: public Task<Create, commands::create::In>
{
public:
    using Task::Task;
    void run(const commands::create::In& in);
};

}
