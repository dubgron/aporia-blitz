#include "aporia_debug.hpp"

#include <chrono>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#if defined(__cpp_lib_format)
    #include <format>
#else
    #include <ctime>
    // Workaround for compilers which don't support std::format
    std::string format_timestamp(std::string_view fmt, const std::chrono::system_clock::time_point& timestamp)
    {
        std::time_t now_c = std::chrono::system_clock::to_time_t(timestamp);
        std::tm now_tm = *std::localtime(&now_c);

        char buff[30];
        strftime(buff, sizeof(buff), fmt.data(), &now_tm);

        return buff;
    }
#endif

namespace Aporia
{
    std::shared_ptr<spdlog::logger> logger;

    void logging_init(const std::string& name)
    {
#if defined(__cpp_lib_format)
        static const std::chrono::zoned_time timestamp{ std::chrono::current_zone(), std::chrono::system_clock::now() };
        static const std::string logfile = std::format("logs/{0:%Y-%m-%d_%H-%M-%OS}.log", timestamp);
#else
        static const std::string logfile = format_timestamp("logs/%Y-%m-%d_%H-%M-%OS.log", std::chrono::system_clock::now());
#endif

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile);

        logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{ std::move(console_sink), std::move(file_sink) });
        logger->set_pattern("[%Y-%m-%d %H:%M:%S:%e] [" + name + "] [%^%l%$] (%s:%!@%#) : %v");
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug);
    }
}
