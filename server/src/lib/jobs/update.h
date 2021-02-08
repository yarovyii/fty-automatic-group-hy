#pragma once
#include "lib/task.h"

namespace fty::job {

class Update: public Task<Update, commands::update::In, commands::update::Out>
{
public:
    using Task::Task;
    void run(const commands::update::In& in, commands::update::Out& out);
};

}
