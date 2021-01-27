#include "common/group.h"

namespace fty {

std::ostream& operator<<(std::ostream& ss, fty::Group::ConditionOp value)
{
    ss << [&]() {
        switch (value) {
        case fty::Group::ConditionOp::Between:
            return "Between";
        case fty::Group::ConditionOp::Contains:
            return "Contains";
        case fty::Group::ConditionOp::Equal:
            return "Equal";
        case fty::Group::ConditionOp::In:
            return "In";
        case fty::Group::ConditionOp::Less:
            return "Less";
        case fty::Group::ConditionOp::LessEqual:
            return "LessEqual";
        case fty::Group::ConditionOp::More:
            return "More";
        case fty::Group::ConditionOp::MoreEqual:
            return "MoreEqual";
        }
        return "Unknown";
    }();
    return ss;
}

std::istream& operator>>(std::istream& ss, fty::Group::ConditionOp& value)
{
    std::string strval;
    ss >> strval;
    if (strval == "Between") {
        value = fty::Group::ConditionOp::Between;
    } else if (strval == "Contains") {
        value = fty::Group::ConditionOp::Contains;
    } else if (strval == "Equal") {
        value = fty::Group::ConditionOp::Equal;
    } else if (strval == "In") {
        value = fty::Group::ConditionOp::In;
    } else if (strval == "Less") {
        value = fty::Group::ConditionOp::Less;
    } else if (strval == "LessEqual") {
        value = fty::Group::ConditionOp::LessEqual;
    } else if (strval == "More") {
        value = fty::Group::ConditionOp::More;
    } else if (strval == "MoreEqual") {
        value = fty::Group::ConditionOp::MoreEqual;
    }
    return ss;
}

std::ostream& operator<<(std::ostream& ss, fty::Group::LogicalOp value)
{
    ss << [&]() {
        switch (value) {
        case fty::Group::LogicalOp::And:
            return "AND";
        case fty::Group::LogicalOp::Or:
            return "OR";
        }
        return "n";
    }();
    return ss;
}

std::istream& operator>>(std::istream& ss, fty::Group::LogicalOp& value)
{
    std::string strval;
    ss >> strval;
    if (strval == "AND") {
        value = fty::Group::LogicalOp::And;
    } else if (strval == "OR") {
        value = fty::Group::LogicalOp::Or;
    }
    return ss;
}

}
