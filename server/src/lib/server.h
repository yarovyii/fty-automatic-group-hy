#pragma once
#include "common/message-bus.h"
#include <fty_common_dto.h>
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
    void srrProcess(const Message& msg);
    void doStop();
    void reloadConfig();

private:
    MessageBus m_bus;
    ThreadPool m_pool;

    Slot<> m_stopSlot       = {&Server::doStop, this};
    Slot<> m_loadConfigSlot = {&Server::reloadConfig, this};
};

} // namespace fty
