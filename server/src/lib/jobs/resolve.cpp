#include "resolve.h"
#include "asset/asset-db.h"
#include "asset/db.h"
#include "lib/storage.h"

namespace fty::job {

using namespace fmt::literals;

static std::string sqlOperator(const Group::ConditionOp& op, const std::string& value)
{
    switch (op) {
        case Group::ConditionOp::Contains:
            return "like '%{}%'"_format(value);
        case Group::ConditionOp::Is:
            return "= '{}'"_format(value);
        case Group::ConditionOp::IsNot:
            return "<> '{}'"_format(value);
    }
    return "unknown";
}

static std::string sqlLogicalOperator(const Group::LogicalOp& op)
{
    switch (op) {
        case Group::LogicalOp::And:
            return "AND";
        case Group::LogicalOp::Or:
            return "OR";
    }
    return "unknown";
}

static std::string byName(const Group::Condition& cond)
{
    return "SELECT id_asset_element FROM t_bios_asset_ext_attributes WHERE keytag='name' AND value {}"_format(sqlOperator(cond.op, cond.value));
}

static std::string byLocation(tnt::Connection& conn, const Group::Condition& cond)
{
    auto query = R"(
        SELECT
            id_asset_element
        FROM
            t_bios_asset_element
        WHERE name {})"_format(sqlOperator(cond.op, cond.value));

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


    std::vector<std::string> subQueries;
    for (const auto& cond : group->rules.conditions) {
        if (cond.field == "name") {
            subQueries.push_back(byName(cond));
        } else if (cond.field == "location") {
            subQueries.push_back(byLocation(conn, cond));
        } else {
            throw Error("Unsupported field '{}' in condition", cond.field.value());
        }
    }

    std::string sql = R"(
        SELECT
            id_asset_element as id,
            name
        FROM t_bios_asset_element
        WHERE id_asset_element IN ({})
    )"_format(fty::implode(subQueries, ") " + sqlLogicalOperator(group->rules.op) + " id_asset_element IN ("));

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
