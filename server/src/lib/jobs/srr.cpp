#include "srr.h"
#include "common/logger.h"
#include "common/message.h"
#include "srr/save.h"
#include "srr/restore.h"
#include "srr/reset.h"

#include <fty_common_messagebus.h>
#include <fty_srr_dto.h>

namespace fty::job {

void SrrProcess::run()
{
    using namespace dto;
    using namespace dto::srr;

    if(!m_bus) {
        logError("No messagebus instance available");
    }

    dto::srr::SrrQueryProcessor srrProcessor;

    srrProcessor.saveHandler    = std::bind(&job::srr::save, std::placeholders::_1);
    srrProcessor.restoreHandler = std::bind(&job::srr::restore, std::placeholders::_1);
    srrProcessor.resetHandler   = std::bind(&job::srr::reset, std::placeholders::_1);

    Message resp;
    resp.meta.subject = m_in.meta.subject;
    resp.meta.to      = m_in.meta.replyTo;
    resp.meta.replyTo = m_in.meta.to;

    logDebug("Processing SRR request: {}", m_in.meta.subject.value());

    try {
        // Get request
        auto legacyMsg = m_in.toMessageBus();
        UserData data = legacyMsg.userData();
        Query    query;

        data >> query;

        messagebus::UserData respData;
        respData << (srrProcessor.processQuery(query));

        const auto subject = m_in.meta.subject;
        const auto replyTo = m_in.meta.replyTo;

        for(const auto& el : respData) {
            resp.userData.append(el);
        }

        resp.meta.status = Message::Status::Ok;

        if (auto ans = m_bus->reply(replyTo, m_in, resp); !ans) {
            throw Error("Sending reply failed");
        }
    } catch (const Error& err) {
            logError("Error: {}", err.what());
            resp.meta.status = Message::Status::Error;
            resp.userData.append(err.what());
            if (auto res = m_bus->reply(fty::Channel, m_in, resp); !res) {
                logError(res.error());
            }
        }
}

void SrrProcess::operator()()
{
    run();
}

}
