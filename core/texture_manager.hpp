#pragma once

#include <map>
#include <memory>
#include <string>

#include "logger.hpp"
#include "components/texture.hpp"
#include "configs/texture_config.hpp"

namespace Aporia
{
    class TextureManager final
    {
    public:
        TextureManager(Logger& logger);

        bool load_textures(const TextureConfig& config);

        const Texture& get_texture(const std::string& name) const;

    private:
        std::map<std::string, Texture> _textures;

        Logger& _logger;
    };
}
