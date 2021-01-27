#include "update.h"
#include "lib/storage.h"

namespace fty::job {

void Update::run(const commands::update::In& in)
{
    if (auto ret = Storage::save(in); !ret) {
        throw Error(ret.error());
    }
}

}
