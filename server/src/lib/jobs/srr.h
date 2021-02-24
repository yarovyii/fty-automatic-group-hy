#pragma once

#include <common/commands.h>
#include <common/message-bus.h>
#include <fty/expected.h>
#include <fty_common_messagebus.h>
#include <fty_srr_dto.h>
#include <mutex>

namespace fty::job::srr {

class SrrHandler
{
public:
    SrrHandler(const std::string& clientName)
        : m_clientName(clientName){};

    [[nodiscard]] Expected<void> run();

    void onMessage(const messagebus::Message& msg);

    dto::srr::SaveResponse    save(const dto::srr::SaveQuery& query);
    dto::srr::RestoreResponse restore(const dto::srr::RestoreQuery& query);
    dto::srr::ResetResponse   reset(const dto::srr::ResetQuery& query);

private:
    std::string m_clientName;

    std::unique_ptr<messagebus::MessageBus> m_srrBus;
    dto::srr::SrrQueryProcessor             m_srrProcessor;

    std::string     m_busName;
    fty::MessageBus m_bus;

    std::mutex m_srrLock;

    fty::Expected<fty::commands::list::Out> list();
    fty::Expected<fty::commands::read::Out> read(const uint64_t id);
    fty::Expected<void>                     create(const fty::Group& group);
    fty::Expected<void>                     remove(const uint64_t id);
    fty::Expected<void>                     clear();
};

} // namespace fty::job::srr
