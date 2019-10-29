#include "logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Aporia
{
    Logger::Logger(const std::string& name)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink.txt");

        _logger = std::make_shared<spdlog::logger>(spdlog::logger(name, { std::move(console_sink), std::move(file_sink) }));
        _logger->set_pattern("[%Y-%m-%d %H:%M:%S] [" + name + "] [%^%l%$] (%s:%!@%#) : %v");
        _logger->set_level(spdlog::level::err);
        _logger->flush_on(spdlog::level::debug);
    }

    LogBuffer Logger::log(spdlog::level::level_enum lvl, const char* file, int line, const char* func)
    {
        return LogBuffer(_logger, lvl, file, line, func);
    }
}
