#include "update.h"
#include "lib/storage.h"
#include "lib/mutex.h"

namespace fty::job {

void Update::run(const commands::update::In& in, commands::update::Out& out)
{
    if (auto ret = in.check(); !ret) {
        throw Error(ret.error());
    }

    if (Storage::byName(in.name)) {
        throw Error("Group with name '{}' already exists", in.name.value());
    }
    
    fty::storage::Mutex::Write mx;
    std::lock_guard<fty::storage::Mutex::Write> lock(mx);

    if (auto ret = Storage::save(in); !ret) {
        throw Error(ret.error());
    } else {
        out = *ret;
        notify(commands::notify::Updated, ret->id);
    }
}

}
