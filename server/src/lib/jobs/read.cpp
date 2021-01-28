#include "read.h"
#include "lib/storage.h"

namespace fty::job {

void Read::run(const commands::read::In& cmd, commands::read::Out& out)
{
    if (auto it = Storage::byId(cmd)) {
        out = *it;
    } else {
        throw Error(it.error());
    }
}

}
