#pragma once

#include <map>
#include <memory>
#include <string>

#include "logger.hpp"
#include "platform.hpp"
#include "texture.hpp"
#include "configs/texture_config.hpp"

namespace Aporia
{
    class APORIA_API TextureManager final
    {
    public:
        TextureManager(Logger& logger);

        bool load_textures(const TextureConfig& config);

        const std::shared_ptr<Texture>& get_texture(const std::string& name) const;

    private:
        std::map<std::string, std::shared_ptr<Texture>> _textures;

        Logger& _logger;
    };
}
