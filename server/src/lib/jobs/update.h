#pragma once
#include "lib/task.h"

namespace fty::job {

class Update: public Task<Update, commands::update::In>
{
public:
    using Task::Task;
    void run(const commands::update::In& in);
};

}
