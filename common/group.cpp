#include "common/group.h"

namespace fty {

std::ostream& operator<<(std::ostream& ss, fty::Group::ConditionOp value)
{
    ss << [&]() {
        switch (value) {
        case fty::Group::ConditionOp::Contains:
            return "CONTAINS";
        case fty::Group::ConditionOp::Is:
            return "IS";
        case fty::Group::ConditionOp::IsNot:
            return "ISNOT";
        }
        return "Unknown";
    }();
    return ss;
}

std::istream& operator>>(std::istream& ss, fty::Group::ConditionOp& value)
{
    std::string strval;
    ss >> strval;
    if (strval == "CONTAINS") {
        value = fty::Group::ConditionOp::Contains;
    } else if (strval == "IS") {
        value = fty::Group::ConditionOp::Is;
    } else if (strval == "ISNOT") {
        value = fty::Group::ConditionOp::IsNot;
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
