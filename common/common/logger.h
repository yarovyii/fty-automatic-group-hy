#pragma once
#include <fmt/format.h>
#include <fty/translate.h>
#include <fty_log.h>

#define logError(...) _log(log4cplus::ERROR_LOG_LEVEL, __VA_ARGS__)
#define logDebug(...) _log(log4cplus::DEBUG_LOG_LEVEL, __VA_ARGS__)
#define logInfo(...)  _log(log4cplus::INFO_LOG_LEVEL, __VA_ARGS__)
#define logWarn(...)  _log(log4cplus::WARN_LOG_LEVEL, __VA_ARGS__)
#define logFatal(...) _log(log4cplus::FATAL_LOG_LEVEL, __VA_ARGS__)

#define _log(level, ...)                                                                                               \
    ftylog_getInstance()->insertLog(level, __FILE__, __LINE__, __func__, fty::logger::format(__VA_ARGS__).c_str())

namespace fty::logger {

inline std::string format(const std::string& str)
{
    return str;
}

template <typename... Args>
inline std::string format(const std::string& str, const Args&... args)
{
    try {
        return fmt::format(str, args...);
    } catch (const fmt::format_error& /*err*/) {
        return str;
    }
}

template <typename... Args>
inline std::string format(Translate& trans, const Args&... args)
{
    return trans.format(args...).toString();
}

} // namespace fty::logger
