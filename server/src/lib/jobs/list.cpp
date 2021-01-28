#include "list.h"
#include "lib/storage.h"

namespace fty::job {

void List::run(commands::list::Out& out)
{
    for (const auto& id : Storage::ids()) {
        if (auto el = Storage::byId(id)) {
            auto& info = out.append();
            info.id    = id;
            info.name  = el->name;
        } else {
            throw Error(el.error());
        }
    }
}

} // namespace fty::job
