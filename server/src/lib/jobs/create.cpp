#include "create.h"
#include "lib/storage.h"

namespace fty::job {

void Create::run(const commands::create::In& in, commands::create::Out& out)
{
    if (auto ret = Storage::save(in); !ret) {
        throw Error(ret.error());
    } else {
        out = *ret;
        notify(commands::notify::Created, ret->id);
    }
}

}
