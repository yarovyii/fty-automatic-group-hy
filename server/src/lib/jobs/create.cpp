#include "create.h"
#include "lib/storage.h"

namespace fty::job {

void Create::run(const commands::create::In& in)
{
    if (auto ret = Storage::save(in); !ret) {
        throw Error(ret.error());
    }
}

}
