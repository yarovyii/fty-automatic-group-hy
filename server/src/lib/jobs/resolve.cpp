#include "resolve.h"
#include "asset/asset-db.h"
#include "asset/db.h"
#include "lib/storage.h"
#include <fty_common_asset_types.h>

namespace fty::job {

using namespace fmt::literals;

static std::string op(const Group::Condition& cond)
{
    switch (cond.op) {
        case Group::ConditionOp::Contains:
            return "like";
        case Group::ConditionOp::Is:
            return "=";
        case Group::ConditionOp::IsNot:
            return "<>";
        case Group::ConditionOp::Unknown:
            return "unknown";
    }
    return "unknown";
}

static std::string value(const Group::Condition& cond)
{
    if (cond.op == Group::ConditionOp::Contains) {
        return "%{}%"_format(cond.value.value());
    } else {
        return cond.value.value();
    }
}

static std::string sqlLogicalOperator(const Group::LogicalOp& op)
{
    switch (op) {
        case Group::LogicalOp::And:
            return "AND";
        case Group::LogicalOp::Or:
            return "OR";
        case Group::LogicalOp::Unknown:
            return "unknown";
    }
    return "unknown";
}

static std::string byName(const Group::Condition& cond)
{
    return R"(
        SELECT
            id_asset_element
        FROM
            t_bios_asset_ext_attributes
        WHERE
            keytag='name' AND
            value {} '{}')"_format(op(cond), value(cond));
}

static std::string byContact(const Group::Condition& cond)
{
    std::string sql = R"(
        SELECT
            id_asset_element
        FROM
            t_bios_asset_ext_attributes
        WHERE
            (keytag='device.contact' OR keytag='contact_email') AND
            value {op} '{val}')";

    if (cond.op == Group::ConditionOp::IsNot) {
        sql = R"(
                SELECT
                    id_asset_element
                FROM
                    t_bios_asset_element
                WHERE
                    id_asset_element NOT IN ()" +
              sql + ")";
    }
    return fmt::format(sql, "op"_a = cond.op != Group::ConditionOp::IsNot ? op(cond) : "=", "val"_a = value(cond));
}

static std::string byType(const Group::Condition& cond)
{
    return R"(
        SELECT
            e.id_asset_element
        FROM
            t_bios_asset_element as e
        LEFT JOIN t_bios_asset_element_type as t
            ON e.id_type = t.id_asset_element_type
        WHERE
            t.name {} '{}')"_format(op(cond), value(cond));
}

static std::string bySubType(const Group::Condition& cond)
{
    return R"(
        SELECT
            e.id_asset_element
        FROM
            t_bios_asset_element as e
        LEFT JOIN t_bios_asset_device_type as t
            ON e.id_subtype = t.id_asset_device_type
        WHERE
            t.name {} '{}')"_format(op(cond), value(cond));
}

static std::string byLocation(tnt::Connection& conn, const Group::Condition& cond)
{
    auto query = R"(
        SELECT
            id_asset_element
        FROM
            t_bios_asset_element
        WHERE
            id_type={} AND
            name {} '{}')"_format(persist::DATACENTER, op(cond), value(cond));

    try {
        std::vector<int64_t> ids;
        // Select all ids for location
        for (const auto& row : conn.select(query)) {
            auto elQuery = R"(
                SELECT
                    p.id_asset_element
                FROM
                    v_bios_asset_element_super_parent p
                WHERE
                    :containerid in (p.id_parent1, p.id_parent2, p.id_parent3, p.id_parent4,
                    p.id_parent5, p.id_parent6, p.id_parent7, p.id_parent8, p.id_parent9, p.id_parent10)
            )";

            for (const auto& elRow : conn.select(elQuery, "containerid"_p = row.get<int64_t>("id_asset_element"))) {
                ids.push_back(elRow.get<int64_t>("id_asset_element"));
            }
        }

        if (ids.empty()) {
            return R"(
                SELECT
                    id_asset_element
                FROM
                    t_bios_asset_element
                WHERE id_asset_element = 0
            )";
        }

        return R"(
            SELECT
                id_asset_element
            FROM
                t_bios_asset_element
            WHERE id_asset_element in ({})
        )"_format(fty::implode(ids, ","));
    } catch (const std::exception& e) {
        throw Error(e.what());
    }
}

static std::string byHostName(const Group::Condition& cond)
{
    std::string sql = R"(
        SELECT
            e.id_asset_element
        FROM
            t_bios_asset_element e
        LEFT JOIN
            t_bios_asset_ext_attributes a ON e.id_asset_element = a.id_asset_element
        WHERE
            a.keytag='hostname.1' AND e.id_type = {type} AND
            a.value {op} '{val}')";
    if (cond.op == Group::ConditionOp::IsNot) {
        sql = R"(
                SELECT
                    id_asset_element
                FROM
                    t_bios_asset_element
                WHERE
                     id_type = {type} AND id_asset_element NOT IN ()" +
              sql + ")";
    }
    return fmt::format(sql, "type"_a = persist::DEVICE, "op"_a = cond.op != Group::ConditionOp::IsNot ? op(cond) : "=",
        "val"_a = value(cond));
}

static std::string byIpAddress(const Group::Condition& cond)
{
    std::string sql = R"(
        SELECT
            e.id_asset_element
        FROM
            t_bios_asset_element e
        LEFT JOIN
            t_bios_asset_ext_attributes a ON e.id_asset_element = a.id_asset_element
        WHERE
            a.keytag='ip.1' AND e.id_type = {type} AND
            {val}
    )";

    if (cond.op == Group::ConditionOp::IsNot) {
        sql = R"(
                SELECT
                    id_asset_element
                FROM
                    t_bios_asset_element
                WHERE
                     id_type = {type} AND id_asset_element NOT IN ()" +
              sql + ")";
    }

    std::vector<std::string> conds;
    auto                     addresses = fty::split(cond.value, "|");
    for (const auto& addr : addresses) {
        if (size_t pos = addr.find("*"); pos != std::string::npos) {
            std::string pre = addr.substr(0, pos);
            conds.push_back("a.value LIKE '{}%'"_format(pre));
        } else {
            std::string sop = cond.op == Group::ConditionOp::IsNot ? "=" : op(cond);
            std::string saddr = cond.op == Group::ConditionOp::Contains ? "%" + addr + "%" : addr;
            conds.push_back(
                "a.value {} '{}'"_format(sop, saddr));
        }
    }

    return fmt::format(sql, "type"_a = persist::DEVICE, "op"_a = cond.op != Group::ConditionOp::IsNot ? op(cond) : "=",
        "val"_a = fty::implode(conds, " OR "));
}

static std::string groupSql(tnt::Connection& conn, const Group::Rules& group)
{
    std::vector<std::string> subQueries;
    for (const auto& it : group.conditions) {
        if (it.is<Group::Condition>()) {
            const auto& cond = it.get<Group::Condition>();
            switch (cond.field) {
                case Group::Fields::Contact:
                    subQueries.push_back(byContact(cond));
                    break;
                case Group::Fields::HostName:
                    subQueries.push_back(byHostName(cond));
                    break;
                case Group::Fields::IPAddress:
                    subQueries.push_back(byIpAddress(cond));
                    break;
                case Group::Fields::Location:
                    subQueries.push_back(byLocation(conn, cond));
                    break;
                case Group::Fields::Name:
                    subQueries.push_back(byName(cond));
                    break;
                case Group::Fields::Type:
                    subQueries.push_back(byType(cond));
                    break;
                case Group::Fields::SubType:
                    subQueries.push_back(bySubType(cond));
                    break;
                case Group::Fields::Unknown:
                default:
                    throw Error("Unsupported field '{}' in condition", cond.field.value());
            }
        } else {
            subQueries.push_back(groupSql(conn, it.get<Group::Rules>()));
        }
    }

    if (subQueries.empty()) {
        throw Error("Request is empty");
    }

    std::string sql = R"(
        SELECT
            id_asset_element as id
        FROM t_bios_asset_element
        WHERE id_asset_element IN ({})
    )"_format(fty::implode(subQueries, ") " + sqlLogicalOperator(group.groupOp) + " id_asset_element IN ("));

    return sql;
}

void Resolve::run(const commands::resolve::In& in, commands::resolve::Out& assetList)
{
    logDebug("resolve {}", *pack::json::serialize(in));

    auto group = Storage::byId(in.id);
    if (!group) {
        throw Error(group.error());
    }

    // Normal connect in _this_ thread, otherwise tntdb will fail
    tntdb::connect(getenv("DBURL") ? getenv("DBURL") : DBConn::url);
    // Normal connection, continue my sad work with db
    tnt::Connection conn;

    std::string groups = groupSql(conn, group->rules);

    std::string sql = R"(
        SELECT
            id_asset_element as id,
            name
        FROM t_bios_asset_element
        WHERE id_asset_element IN ({})
        ORDER BY id
    )"_format(groups);

    //std::cerr << sql << std::endl;

    try {
        for (const auto& row : conn.select(sql)) {
            auto& line = assetList.append();
            line.id    = row.get<u_int64_t>("id");
            line.name  = row.get("name");
        }
    } catch (const std::exception& e) {
        throw Error(e.what());
    }
}

} // namespace fty::job
