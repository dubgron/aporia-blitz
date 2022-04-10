#pragma once

#include <memory>
#include <string>

#include <spdlog/logger.h>

#include "log_buffer.hpp"

#define LOG_LEVEL(lvl)  lvl, __FILE__, __LINE__, __func__
#define LOG_DEBUG       LOG_LEVEL(spdlog::level::debug)
#define LOG_INFO        LOG_LEVEL(spdlog::level::info)
#define LOG_WARNING     LOG_LEVEL(spdlog::level::warn)
#define LOG_ERROR       LOG_LEVEL(spdlog::level::err)
#define LOG_CRITICAL    LOG_LEVEL(spdlog::level::critical)

namespace Aporia
{
    class Logger final
    {
    public:
        Logger(const std::string& name);

        LogBuffer log(spdlog::level::level_enum lvl, const char* file, int line, const char* func) const;

    private:
        std::shared_ptr<spdlog::logger> _logger;
    };
}
