#pragma once

#include <memory>
#include <string_view>

#include <spdlog/logger.h>

#if defined(APORIA_DEBUGTOOLS)
    #define APORIA_LOG(lvl, ...) Aporia::Logger::Log(__FILE__, __LINE__, __func__, lvl, __VA_ARGS__)
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
        Logger() = delete;

        static void Init(const std::string& name);

        static void Log(const char* file, i32 line, const char* func, LogLevel lvl, std::string_view msg);

        template<typename... Args>
        static inline void Log(const char* file, i32 line, const char* func, LogLevel lvl, std::string_view fmt, Args&&... args)
        {
            _logger->log(spdlog::source_loc{ file, line, func }, static_cast<spdlog::level::level_enum>(lvl), fmt, std::forward<Args>(args)...);
        }

    private:
        static std::shared_ptr<spdlog::logger> _logger;
    };
}
