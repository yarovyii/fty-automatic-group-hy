#include "update.h"
#include "lib/storage.h"

namespace fty::job {

void Update::run(const commands::update::In& in, commands::update::Out& out)
{
    if (auto ret = in.check(); !ret) {
        throw Error(ret.error());
    }

    if (auto ret = Storage::save(in); !ret) {
        throw Error(ret.error());
    } else {
        out = *ret;
        notify(commands::notify::Updated, ret->id);
    }
}

}
