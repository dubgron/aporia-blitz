#pragma once

#include <map>
#include <memory>
#include <string>

#include <SFML/Graphics/Texture.hpp>

#include "configs/texture_config.hpp"
#include "logger.hpp"
#include "platform.hpp"
#include "texture.hpp"

namespace Aporia
{
    class APORIA_API TextureManager final
    {
    public:
        TextureManager(const std::shared_ptr<Logger>& logger);

        bool load_textures(const TextureData& config);

        const std::shared_ptr<Texture>& get_texture(const std::string& name) const;

    private:
        std::map<std::string, std::shared_ptr<Texture>> _textures;

        std::shared_ptr<Logger> _logger;
    };
}
