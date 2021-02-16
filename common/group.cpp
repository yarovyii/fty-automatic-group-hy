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

std::ostream& operator<<(std::ostream& ss, fty::Group::Fields value)
{
    ss << [&]() {
        switch (value) {
        case fty::Group::Fields::Contact:
            return "contact";
        case fty::Group::Fields::HostName:
            return "host-name";
        case fty::Group::Fields::IPAddress:
            return "ip-address";
        case fty::Group::Fields::Location:
            return "location";
        case fty::Group::Fields::Name:
            return "name";
        case fty::Group::Fields::Type:
            return "type";
        case fty::Group::Fields::Unknown:
            return "unknown";
        }
        return "unknown";
    }();
    return ss;
}

std::istream& operator>>(std::istream& ss, fty::Group::Fields& value)
{
    std::string strval;
    ss >> strval;
    if (strval == "unknown") {
        value = fty::Group::Fields::Unknown;
    } else if (strval == "contact") {
        value = fty::Group::Fields::Contact;
    } else if (strval == "host-name") {
        value = fty::Group::Fields::HostName;
    } else if (strval == "ip-address") {
        value = fty::Group::Fields::IPAddress;
    } else if (strval == "location") {
        value = fty::Group::Fields::Location;
    } else if (strval == "name") {
        value = fty::Group::Fields::Name;
    } else if (strval == "type") {
        value = fty::Group::Fields::Type;
    }
    return ss;
}

}
