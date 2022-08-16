#pragma once

#include <unordered_map>
#include <string>

#include "components/texture.hpp"

namespace Aporia
{
    class EventManager;
    struct TextureConfig;

    class TextureManager final
    {
    public:
        TextureManager(EventManager& event_manager, const TextureConfig& config);

        const Texture& get_texture(const std::string& name) const;

    private:
        void _load();

    private:
        const TextureConfig& _config;

        std::unordered_map<std::string, Texture> _textures;
    };
}
