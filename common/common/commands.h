#pragma once
#include "group.h"

namespace fty {

static constexpr const char* Channel = "FTY.Q.GROUP.QUERY";

namespace commands::create {
    static constexpr const char* Subject = "CREATE";

    using In = Group;
} // namespace commands::create

namespace commands::update {
    static constexpr const char* Subject = "UPDATE";

    using In = Group;
} // namespace commands::update

namespace commands::remove {
    static constexpr const char* Subject = "DELETE";

    using In = pack::UInt64;
} // namespace commands::remove

namespace commands::resolve {
    static constexpr const char* Subject = "RESOLVE";

    using In  = pack::UInt64;
    using Out = pack::StringList;
} // namespace commands::resolve

namespace commands::list {
    static constexpr const char* Subject = "LIST";

    struct Answer : public pack::Node
    {
        pack::UInt64 id   = FIELD("id");
        pack::String name = FIELD("name");

        using pack::Node::Node;
        META(Answer, id, name);
    };

    using Out = pack::ObjectList<Answer>;
} // namespace commands::list

namespace commands::read {
    static constexpr const char* Subject = "READ";

    using In  = pack::UInt64;
    using Out = Group;
} // namespace commands::read

} // namespace fty
