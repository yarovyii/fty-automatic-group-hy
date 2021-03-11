#pragma once
#include "asset/asset-db.h"
#include "asset/db.h"
#include <fty_common_asset_types.h>

namespace fty {

struct TestDb;

// =====================================================================================================================

struct GroupsDB
{
    ~GroupsDB();

    static Expected<void> init();
    static GroupsDB&      instance();
    static void           destroy();

    bool                    inited = false;
    std::unique_ptr<TestDb> db;

private:
    Expected<void> _init();
};

// =====================================================================================================================

struct SampleDb
{
    SampleDb(const std::string& data);
    ~SampleDb();

    uint32_t idByName(const std::string& name);
private:
    std::vector<uint32_t>           m_ids;
    std::map<std::string, uint32_t> m_mapping;
    std::vector<int64_t>            m_links;
};

// =====================================================================================================================

} // namespace fty
