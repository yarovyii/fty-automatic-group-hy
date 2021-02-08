#include "remove.h"
#include "lib/storage.h"

namespace fty::job {

void Remove::run(const commands::remove::In& in, commands::remove::Out& out)
{
    for(const auto& id: in) {
        auto& line = out.append();
        if (auto ret = Storage::remove(id); !ret) {
            line.append(fty::convert<std::string>(id), ret.error());
        } else {
            line.append(fty::convert<std::string>(id), "Ok");
            pack::UInt64 send;
            send = id;
            notify(commands::notify::Deleted, send);
        }
    }
}

}
