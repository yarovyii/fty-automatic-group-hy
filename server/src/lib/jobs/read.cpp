#include "read.h"
#include "lib/storage.h"
#include "lib/mutex.h"

namespace fty::job {

void Read::run(const commands::read::In& cmd, commands::read::Out& out)
{
    fty::storage::Mutex::Read mx;
    std::lock_guard<fty::storage::Mutex::Read> lock(mx);
    if (auto it = Storage::byId(cmd.id)) {
        out = *it;
    } else {
        throw Error(it.error());
    }
}

}
