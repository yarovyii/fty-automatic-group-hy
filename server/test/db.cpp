#include "db.h"
#include <asset/test-db.h>

namespace fty {

GroupsDB::GroupsDB()
    : dc("datacenter")
    , dc1("datacenter1")
    , srv1("srv1", dc, "srv1")
    , srv2("srv2", dc, "srv2")
    , srv3("srv3", dc, "srv3")
    , srv11("srv11", dc1, "srv11")
    , srv21("srv21", dc1, "srv21")
{
}

GroupsDB::~GroupsDB()
{
    destroy();
}

fty::Expected<void> GroupsDB::init()
{
    return instance()._init();
}

GroupsDB& GroupsDB::instance()
{
    static GroupsDB inst;
    return inst;
}

void GroupsDB::destroy()
{
    if (instance().inited) {
        instance().srv1.remove();
        instance().srv2.remove();
        instance().srv3.remove();
        instance().srv11.remove();
        instance().srv21.remove();
        instance().dc.remove();
        instance().dc1.remove();

        instance().db->destroy();
        instance().inited = false;
    }
}

Expected<void> GroupsDB::_init()
{
    if (inited) {
        return {};
    }

    db = std::make_unique<TestDb>();

    if (auto res = db->create()) {
        std::string url = *res;
        setenv("DBURL", url.c_str(), 1);
    } else {
        return fty::unexpected(res.error());
    }

    // Db
    if (auto ret = dc.create(); !ret) {
        return unexpected(ret.error());
    }
    if (auto ret = dc1.create(); !ret) {
        return unexpected(ret.error());
    }

    if (auto ret = srv1.create(); !ret) {
        return unexpected(ret.error());
    }
    if (auto ret = srv2.create(); !ret) {
        return unexpected(ret.error());
    }
    if (auto ret = srv3.create(); !ret) {
        return unexpected(ret.error());
    }

    if (auto ret = srv11.create(); !ret) {
        return unexpected(ret.error());
    }
    if (auto ret = srv21.create(); !ret) {
        return unexpected(ret.error());
    }

    srv11.setExtAttributes({{"device.contact", "dim"}, {"hostname.1", "localhost"}});
    srv21.setExtAttributes({{"contact_email", "dim@eaton.com"}});
    srv1.setExtAttributes({{"ip.1", "127.0.0.1"}});
    srv2.setExtAttributes({{"ip.1", "192.168.0.1"}});

    inited = true;
    return {};
}

} // namespace fty
