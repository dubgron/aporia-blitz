#include "engine.hpp"

namespace Aporia
{
    Engine::Engine(const std::string& config_file)
        : _configs(config_file)
    {
    }
}