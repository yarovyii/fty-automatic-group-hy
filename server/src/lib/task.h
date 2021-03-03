#pragma once
#include "common/commands.h"
#include "common/logger.h"
#include "common/message-bus.h"
#include "common/message.h"
#include "config.h"
#include <fty/expected.h>
#include <fty/thread-pool.h>

namespace fty::job {

// =====================================================================================================================

class Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
    template <typename... Args>
    Error(const std::string& fmt, const Args&... args)
        : std::runtime_error(formatString(fmt, args...))
    {
    }

private:
    template <typename... Args>
    static std::string formatString(const std::string& msg, const Args&... args)
    {
        try {
            return fmt::format(msg, args...);
        } catch (const fmt::format_error&) {
            return msg;
        }
    }
};

// =====================================================================================================================

/// Basic responce wrapper
template <typename T>
class Response : public pack::Node
{
public:
    pack::String                error   = FIELD("error");
    pack::Enum<Message::Status> status  = FIELD("status");
    pack::String                subject = FIELD("subject");
    T                           out     = FIELD("out");

public:
    using pack::Node::Node;
    META(Response, error, status, subject, out);

public:
    void setError(const std::string& errMsg)
    {
        error  = errMsg;
        status = Message::Status::Error;
    }

    operator Message()
    {
        Message msg;
        msg.meta.status = status;
        if (subject.hasValue()) {
            msg.meta.subject = subject;
        }

        if (status == Message::Status::Ok) {
            if (out.hasValue()) {
                msg.userData.append(*pack::json::serialize(out));
            } else {
                if constexpr (std::is_base_of_v<pack::IList, T>) {
                    msg.userData.append("[]");
                } else if constexpr (std::is_base_of_v<pack::IMap, T> || std::is_base_of_v<pack::INode, T>) {
                    msg.userData.append("{}");
                } else {
                    msg.userData.append("");
                }
            }
        } else {
            msg.userData.append(error);
        }
        return msg;
    }
};

template <>
class Response<void> : public pack::Node
{
public:
    pack::String                error  = FIELD("error");
    pack::Enum<Message::Status> status = FIELD("status");

public:
    using pack::Node::Node;
    META(Response, error, status);

public:
    void setError(const std::string& errMsg)
    {
        error  = errMsg;
        status = Message::Status::Error;
    }

    operator Message()
    {
        Message msg;
        msg.meta.status = status;
        if (status != Message::Status::Ok) {
            msg.userData.append(error);
        }
        return msg;
    }
};

// =====================================================================================================================

template <typename T, typename InputT, typename ResponseT = void>
class Task : public fty::Task<T>
{
public:
    Task(const Message& in, MessageBus& bus)
        : m_in(in)
        , m_bus(&bus)
    {
    }

    Task() = default;

    void operator()() override
    {
        Response<ResponseT> response;
        try {
            if (auto it = dynamic_cast<T*>(this)) {
                if constexpr (!std::is_same<InputT, void>::value) {
                    if (m_in.userData.empty()) {
                        throw Error("Wrong input data: payload is empty");
                    }

                    InputT cmd;
                    auto ret = pack::json::deserialize(m_in.userData[0], cmd);
                    if (!ret) {
                        throw Error("Wrong input data: format of payload is incorrect");
                    }

                    if constexpr (std::is_same<ResponseT, void>::value) {
                        it->run(cmd);
                    } else {
                        it->run(cmd, response.out);
                    }
                } else if constexpr (!std::is_same<ResponseT, void>::value) {
                    it->run(response.out);
                }
            } else {
                throw Error("Not a correct task");
            }

            response.status = Message::Status::Ok;
            if (auto res = m_bus->reply(fty::Channel, m_in, response); !res) {
                logError(res.error());
            }
        } catch (const Error& err) {
            logError("Error: {}", err.what());
            response.setError(err.what());
            if (auto res = m_bus->reply(fty::Channel, m_in, response); !res) {
                logError(res.error());
            }
        }
    }

    template <typename Payload>
    void notify(const std::string& subject, const Payload& payload)
    {
        if (m_bus) {
            Response<Payload> msg;
            msg.subject = subject;
            msg.out     = payload;

            if (auto ret = m_bus->publish(fty::Events, msg); !ret) {
                logError(ret.error());
            }
        }
    }

protected:
    Message     m_in;
    MessageBus* m_bus = nullptr;
};

} // namespace fty::job
