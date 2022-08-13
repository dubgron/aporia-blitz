#pragma once

#include <memory>
#include <string>

#include <spdlog/logger.h>

#if defined(APORIA_DEBUGTOOLS)
    #define APORIA_LOG(logger, lvl, fmt, ...) logger.log(__FILE__, __LINE__, __func__, lvl, fmt, __VA_ARGS__)
#else
    #define APORIA_LOG
#endif

namespace Aporia
{
    enum LogLevel
    {
        Trace       = spdlog::level::trace,
        Debug       = spdlog::level::debug,
        Info        = spdlog::level::info,
        Warning     = spdlog::level::warn,
        Error       = spdlog::level::err,
        Critical    = spdlog::level::critical,
        Off         = spdlog::level::off
    };

    class Logger final
    {
    public:
        Logger(const std::string& name);

        template<typename... Args>
        void log(const char* file, int line, const char* func, LogLevel lvl, std::string_view fmt, Args&&... args) const
        {
            _logger->log(spdlog::source_loc{ file, line, func }, static_cast<spdlog::level::level_enum>(lvl), fmt, std::forward<Args>(args)...);
        }

    private:
        mutable std::shared_ptr<spdlog::logger> _logger;
    };
}
