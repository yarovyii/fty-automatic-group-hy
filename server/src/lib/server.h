#pragma once
#include "common/message-bus.h"
#include "jobs/srr.h"
#include <fty/event.h>
#include <fty/thread-pool.h>

namespace fty {

class Server
{
public:
    [[nodiscard]] Expected<void> run();
    void                         shutdown();
    void                         wait();

    Event<> stop;

private:
    void process(const Message& msg);
    void handleSrr(const messagebus::Message& msg);
    void doStop();
    void reloadConfig();

private:
    MessageBus m_bus;
    ThreadPool m_pool;

    std::shared_ptr<job::srr::SrrHandler> m_srrHandlerPtr;

    Slot<> m_stopSlot       = {&Server::doStop, this};
    Slot<> m_loadConfigSlot = {&Server::reloadConfig, this};
};

} // namespace fty
