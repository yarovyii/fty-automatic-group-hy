#include "remove.h"
#include "lib/storage.h"

namespace fty::job {

void Remove::run(const commands::remove::In& in)
{
    if (auto ret = Storage::remove(in); !ret) {
        throw Error(ret.error());
    }
}

}
