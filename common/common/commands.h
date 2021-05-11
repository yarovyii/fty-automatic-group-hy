#pragma once
#include "group.h"

namespace fty {

static constexpr const char* Channel = "FTY.Q.GROUP.QUERY";
static constexpr const char* Events  = "FTY.T.GROUP.EVENT";

namespace commands::create {
    static constexpr const char* Subject = "CREATE";

    using In  = Group;
    using Out = Group;
} // namespace commands::create

namespace commands::update {
    static constexpr const char* Subject = "UPDATE";

    using In  = Group;
    using Out = Group;
} // namespace commands::update

namespace commands::remove {
    static constexpr const char* Subject = "DELETE";

    using In  = pack::UInt64List;
    using Out = pack::ObjectList<pack::StringMap>;
} // namespace commands::remove

namespace commands::resolve {
    static constexpr const char* Subject = "RESOLVE";

    struct Request : public pack::Node
    {
        pack::UInt64 id = FIELD("id");
        fty::Group::Rules rules = FIELD("rules");
        
        using pack::Node::Node;
        META(Request, id, rules);
    };

    struct Answer : public pack::Node
    {
        pack::UInt64 id   = FIELD("id");
        pack::String name = FIELD("name");

        using pack::Node::Node;
        META(Answer, id, name);
    };


    using In  = Request;
    using Out = pack::ObjectList<Answer>;
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

    struct Request : public pack::Node
    {
        pack::UInt64 id = FIELD("id");

        using pack::Node::Node;
        META(Request, id);
    };

    using In  = Request;
    using Out = Group;
} // namespace commands::read

namespace commands::notify {
    static constexpr const char* Created = "CREATED";
    static constexpr const char* Updated = "UPDATED";
    static constexpr const char* Deleted = "DELETED";
    using Payload                        = pack::UInt64;
} // namespace commands::notify

} // namespace fty
