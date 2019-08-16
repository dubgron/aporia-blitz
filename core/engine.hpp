#pragma once

#include <string>

#include "config_manager.hpp"
#include "platform.hpp"

namespace Aporia
{
    class APORIA_API Engine
    {
    public:
        Engine(const std::string& config_file);

    private:
        ConfigManager _configs;
    };
}
