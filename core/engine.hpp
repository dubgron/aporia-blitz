#pragma once

#include <string>

#include "config.hpp"
#include "platform.hpp"

namespace Aporia
{
    class APORIA_API Engine
    {
    public:
        Engine(const std::string& config);

    private:
        Config _config;
    };
}
