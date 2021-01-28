#include "server.h"
#include "daemon.h"
#include "config.h"
#include "common/logger.h"
#include "jobs/create.h"
#include "jobs/update.h"
#include "jobs/remove.h"
#include "jobs/list.h"
#include "jobs/read.h"
#include "jobs/resolve.h"

namespace fty {

Expected<void> Server::run()
{
    m_stopSlot.connect(Daemon::instance().stopEvent);
    m_loadConfigSlot.connect(Daemon::instance().loadConfigEvent);

    if (auto res = m_bus.init(Config::instance().actorName); !res) {
        return unexpected(res.error());
    }

    if (auto sub = m_bus.subsribe(fty::Channel, &Server::process, this); !sub) {
        return unexpected(sub.error());
    }

    return {};
}

void Server::wait()
{
    stop.wait();
}

void Server::doStop()
{
    stop();
}

void Server::reloadConfig()
{
    Config::instance().reload();
}

void Server::process(const Message& msg)
{
    logDebug("Discovery: got message {}", msg.dump());
    if (msg.meta.subject == commands::create::Subject) {
        m_pool.pushWorker<job::Create>(msg, m_bus);
    } else if (msg.meta.subject == commands::update::Subject) {
        m_pool.pushWorker<job::Update>(msg, m_bus);
    } else if (msg.meta.subject == commands::remove::Subject) {
        m_pool.pushWorker<job::Remove>(msg, m_bus);
    } else if (msg.meta.subject == commands::list::Subject) {
        m_pool.pushWorker<job::List>(msg, m_bus);
    } else if (msg.meta.subject == commands::read::Subject) {
        m_pool.pushWorker<job::Read>(msg, m_bus);
    } else if (msg.meta.subject == commands::resolve::Subject) {
        m_pool.pushWorker<job::Resolve>(msg, m_bus);
    }
}

void Server::shutdown()
{
    stop();
    m_pool.stop();
}

} // namespace fty
