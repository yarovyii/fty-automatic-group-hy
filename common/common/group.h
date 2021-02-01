#pragma once
#include <pack/pack.h>

namespace fty {

struct Group : public pack::Node
{
    enum class LogicalOp
    {
        Or,
        And
    };

    enum class ConditionOp
    {
        Contains,
        Is,
        IsNot
    };

    struct Condition : public pack::Node
    {
        pack::String            field  = FIELD("field");
        pack::Enum<ConditionOp> op     = FIELD("operator");
        pack::String            entity = FIELD("Entity");

        using pack::Node::Node;
        META(Condition, field, op, entity);
    };

    struct Rules : public pack::Node
    {
        pack::Enum<LogicalOp>       op         = FIELD("operator");
        pack::ObjectList<Condition> conditions = FIELD("conditions");

        using pack::Node::Node;
        META(Rules, op, conditions);
    };

    pack::UInt64 id    = FIELD("id");
    pack::String name  = FIELD("name");
    Rules        rules = FIELD("rules");

    using pack::Node::Node;
    META(Group, id, name, rules);
};

std::ostream& operator<<(std::ostream& ss, fty::Group::ConditionOp value);
std::istream& operator>>(std::istream& ss, fty::Group::ConditionOp& value);
std::ostream& operator<<(std::ostream& ss, fty::Group::LogicalOp value);
std::istream& operator>>(std::istream& ss, fty::Group::LogicalOp& value);

} // namespace fty

