#include "storage.h"
#include "config.h"
#include <iostream>
#include <mutex>
#include <filesystem>

namespace fty {

struct DbObj : public pack::Node
{
    pack::UInt64            lastId = FIELD("last-id");
    pack::ObjectList<Group> groups = FIELD("groups");

    using pack::Node::Node;
    META(DbObj, lastId, groups);
};

class Storage::Impl
{
public:
    explicit Impl(const std::string& dbpath)
        : m_dbpath(dbpath)
    {
    }

    bool inited() const
    {
        return m_inited;
    }

    Expected<void> init()
    {
        m_inited = true;

        std::filesystem::path path(m_dbpath);
        std::filesystem::create_directories(path.parent_path());

        std::cerr << "load " << m_dbpath << std::endl;
        if (auto ret = pack::yaml::deserializeFile(m_dbpath, db); !ret) {
            return unexpected(ret.error());
        }
        return {};
    }

    Expected<void> save()
    {
        if (auto ret = pack::yaml::serializeFile(m_dbpath, db); !ret) {
            return unexpected(ret.error());
        }
        return {};
    }

public:
    std::mutex mutex;
    DbObj      db;

private:
    std::string m_dbpath;
    bool        m_inited = false;
};

Storage& Storage::instance()
{
    static Storage inst;
    return inst;
}

Storage::Storage()
    : m_impl(new Impl(Config::instance().dbpath))
{
}

Storage::~Storage()
{
}

Expected<Group> Storage::byName(const std::string& name)
{
    auto&                       db = instance();
    std::lock_guard<std::mutex> guard(db.m_impl->mutex);

    if (!db.m_impl->inited()) {
        db.m_impl->init();
    }

    auto found = db.m_impl->db.groups.find([&](const auto& rule) {
        return rule.name == name;
    });

    if (found) {
        return *found;
    }

    return unexpected("Not found");
}

Expected<Group> Storage::byId(uint64_t id)
{
    auto&                       db = instance();
    std::lock_guard<std::mutex> guard(db.m_impl->mutex);

    if (!db.m_impl->inited()) {
        db.m_impl->init();
    }

    auto found = db.m_impl->db.groups.find([&](const auto& rule) {
        return rule.id == id;
    });

    if (found) {
        return *found;
    }

    return unexpected("Not found");
}

std::vector<std::string> Storage::names()
{
    auto&                       db = instance();
    std::lock_guard<std::mutex> guard(db.m_impl->mutex);

    if (!db.m_impl->inited()) {
        db.m_impl->init();
    }

    std::vector<std::string> ret;

    for (const auto& group : db.m_impl->db.groups) {
        ret.push_back(group.name);
    }

    return ret;
}

std::vector<uint64_t> Storage::ids()
{
    auto&                       db = instance();
    std::lock_guard<std::mutex> guard(db.m_impl->mutex);

    if (!db.m_impl->inited()) {
        db.m_impl->init();
    }

    std::vector<uint64_t> ret;

    for (const auto& group : db.m_impl->db.groups) {
        ret.push_back(group.id);
    }

    return ret;
}

Expected<Group> Storage::save(const Group& group)
{
    auto&                       db = instance();
    std::lock_guard<std::mutex> guard(db.m_impl->mutex);

    if (!db.m_impl->inited()) {
        db.m_impl->init();
    }

    Group toSave = group;

    if (!toSave.id.hasValue()) {
        db.m_impl->db.lastId = db.m_impl->db.lastId + 1;
        toSave.id            = db.m_impl->db.lastId.value();
        db.m_impl->db.groups.append(toSave);
    } else {
        auto found = db.m_impl->db.groups.findIndex([&](const auto& rule) {
            return rule.id == toSave.id;
        });
        if (found >= 0) {
            db.m_impl->db.groups[found] = toSave;
        }
    }

    if (auto ret = db.m_impl->save(); !ret) {
        return unexpected(ret.error());
    }

    return std::move(toSave);
}

Expected<void> Storage::removeByName(const std::string& groupName)
{
    auto&                       db = instance();
    std::lock_guard<std::mutex> guard(db.m_impl->mutex);

    if (!db.m_impl->inited()) {
        db.m_impl->init();
    }

    db.m_impl->db.groups.remove([&](const auto& group) {
        return group.name == groupName;
    });

    if (auto ret = db.m_impl->save(); !ret) {
        return unexpected(ret.error());
    }

    return {};
}

Expected<void> Storage::remove(uint64_t id)
{
    auto&                       db = instance();
    std::lock_guard<std::mutex> guard(db.m_impl->mutex);

    if (!db.m_impl->inited()) {
        db.m_impl->init();
    }

    bool removed = db.m_impl->db.groups.remove([&](const auto& group) {
        return group.id == id;
    });

    if (removed) {
        if (auto ret = db.m_impl->save(); !ret) {
            return unexpected(ret.error());
        }
    } else {
        return unexpected("Id '{}' was not found", id);
    }

    return {};
}


} // namespace fty
