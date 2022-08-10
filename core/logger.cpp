#include "logger.hpp"

#include <chrono>
#include <format>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Aporia
{
    Logger::Logger(const std::string& name)
    {
        std::string logfile = std::format("logs/{0:%Y-%m-%d_%H-%M-%OS}.log", std::chrono::system_clock::now());

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile);

        _logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{ std::move(console_sink), std::move(file_sink) });
        _logger->set_pattern("[%Y-%m-%d %H:%M:%S] [" + name + "] [%^%l%$] (%s:%!@%#) : %v");
        _logger->set_level(spdlog::level::debug);
        _logger->flush_on(spdlog::level::debug);
    }

    LogBuffer Logger::log(spdlog::level::level_enum lvl, const char* file, int line, const char* func) const
    {
        return LogBuffer(_logger, lvl, file, line, func);
    }
}
