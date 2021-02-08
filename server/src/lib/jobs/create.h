#pragma once
#include "lib/task.h"

namespace fty::job {

class Create: public Task<Create, commands::create::In, commands::create::Out>
{
public:
    using Task::Task;
    void run(const commands::create::In& in, commands::create::Out& out);
};

}
