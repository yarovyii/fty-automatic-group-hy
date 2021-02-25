#pragma once
#include "asset/asset-db.h"
#include "asset/db.h"
#include <fty_common_asset_types.h>

namespace fty {

struct TestDb;

// =====================================================================================================================

template <uint16_t Type, uint16_t SubType = 0>
class AssetElement : public fty::asset::db::AssetElement
{
public:
    AssetElement(const std::string& _name, const std::string& extName = {})
    {
        name      = _name;
        status    = "active";
        priority  = 1;
        typeId    = Type;
        subtypeId = SubType;
        m_extName = extName;
    }

    AssetElement(const std::string& _name, const asset::db::AssetElement& parent, const std::string& extName = {})
    {
        m_parent = &parent;
        name      = _name;
        status    = "active";
        priority  = 1;
        typeId    = Type;
        subtypeId = SubType;
        m_extName = extName;
    }

    fty::Expected<void> setExtName(const std::string& extName)
    {
        tnt::Connection conn;
        auto            ret = fty::asset::db::insertIntoAssetExtAttributes(conn, id, {{"name", extName}}, true);
        if (!ret) {
            return fty::unexpected(ret.error());
        }
        return {};
    }

    fty::Expected<void> setExtAttributes(const std::map<std::string, std::string>& attr)
    {
        tnt::Connection conn;
        auto            ret = fty::asset::db::insertIntoAssetExtAttributes(conn, id, attr, true);
        if (!ret) {
            return fty::unexpected(ret.error());
        }
        return {};
    }

    fty::Expected<void> create()
    {
        tnt::Connection conn;

        if (m_parent) {
            parentId = m_parent->id;
        }

        if (auto ret = fty::asset::db::insertIntoAssetElement(conn, *this, true); !ret) {
            return fty::unexpected(ret.error());
        } else {
            id = *ret;
        }

        if (!m_extName.empty()) {
            if (auto res = fty::asset::db::insertIntoAssetExtAttributes(conn, id, {{"name", m_extName}}, true); !res) {
                return fty::unexpected(res.error());
            }
        }
        return {};
    }

    fty::Expected<void> remove()
    {
        tnt::Connection conn;

        if (auto ret = fty::asset::db::deleteAssetExtAttributesWithRo(conn, id, true); !ret) {
            return fty::unexpected(ret.error());
        }

        if (auto ret = fty::asset::db::deleteAssetExtAttributesWithRo(conn, id, false); !ret) {
            return fty::unexpected(ret.error());
        }

        if (auto ret = fty::asset::db::deleteAssetElement(conn, id); !ret) {
            return fty::unexpected(ret.error());
        }

        return {};
    }

private:
    std::string m_extName;
    const asset::db::AssetElement* m_parent = nullptr;
};

// =====================================================================================================================

namespace assets {
    using DataCenter = AssetElement<persist::DATACENTER>;
    using Device     = AssetElement<persist::DEVICE>;
    using Feed       = AssetElement<persist::DEVICE, persist::FEED>;
    using Row        = AssetElement<persist::ROW>;
    using Rack       = AssetElement<persist::RACK>;
    using Server     = AssetElement<persist::DEVICE, persist::SERVER>;
} // namespace assets

// =====================================================================================================================

struct GroupsDB
{
    GroupsDB();
    ~GroupsDB();

    static Expected<void> init();
    static GroupsDB&      instance();
    static void           destroy();

    bool                    inited = false;
    std::unique_ptr<TestDb> db;

    assets::DataCenter dc;
    assets::DataCenter dc1;
    assets::Server     srv1;
    assets::Server     srv2;
    assets::Server     srv3;
    assets::Server     srv11;
    assets::Server     srv21;

private:
    Expected<void> _init();
};

// =====================================================================================================================

} // namespace fty
