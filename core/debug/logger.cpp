#include "logger.hpp"

#include <chrono>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#if defined(__cpp_lib_format)
    #include <format>
#else
    #include <ctime>
    /* Workaround for compilers which don't support std::format */
    std::string format_timestamp(std::string fmt, const std::chrono::system_clock::time_point& timestamp)
    {
        std::time_t now_c = std::chrono::system_clock::to_time_t(timestamp);
        std::tm now_tm = *std::localtime(&now_c);

        char buff[30];
        strftime(buff, sizeof(buff), fmt.c_str(), &now_tm);

        return buff;
    }
#endif

namespace Aporia
{
    Logger::Logger(const std::string& name)
    {
        #if defined(__cpp_lib_format)
            static const std::chrono::zoned_time timestamp{ std::chrono::current_zone(), std::chrono::system_clock::now() };
            static const std::string logfile = std::format("logs/{0:%Y-%m-%d_%H-%M-%OS}.log", timestamp);
        #else
            static const std::string logfile = format_timestamp("logs/%Y-%m-%d_%H-%M-%OS.log", std::chrono::system_clock::now());
        #endif

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile);

        _logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{ std::move(console_sink), std::move(file_sink) });
        _logger->set_pattern("[%Y-%m-%d %H:%M:%S] [" + name + "] [%^%l%$] (%s:%!@%#) : %v");
        _logger->set_level(spdlog::level::debug);
        _logger->flush_on(spdlog::level::debug);
    }

    void Logger::log(const char* file, int line, const char* func, LogLevel lvl, std::string_view msg) const
    {
        _logger->log(spdlog::source_loc{ file, line, func }, static_cast<spdlog::level::level_enum>(lvl), msg);
    }
}
