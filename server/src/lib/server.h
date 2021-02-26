#pragma once
#include "common/message-bus.h"
#include <fty_common_dto.h>
#include <fty/event.h>
#include <fty/thread-pool.h>

namespace messagebus {
class Message;
}

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
    void srrProcess(const messagebus::Message& msg);
    void doStop();
    void reloadConfig();

private:
    MessageBus m_bus;
    ThreadPool m_pool;

    dto::srr::SrrQueryProcessor m_srrProcessor;
    std::mutex m_srrLock;

    Slot<> m_stopSlot       = {&Server::doStop, this};
    Slot<> m_loadConfigSlot = {&Server::reloadConfig, this};
};

} // namespace fty
