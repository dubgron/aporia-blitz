#pragma once

#include <string>
#include <unordered_map>

#include "aporia_types.hpp"

namespace Aporia
{
    struct Image
    {
        u8* pixels = nullptr;

        i32 width = 0;
        i32 height = 0;
        i32 channels = 0;

        void load(std::string_view filepath);
        void unload();
    };

    struct Texture
    {
        u32 id = 0;

        i32 width = 0;
        i32 height = 0;
        i32 channels = 0;
    };

    struct SubTexture
    {
        v2 u{ 0.f };
        v2 v{ 0.f };
        Texture source;
    };

    void load_texture_atlas(std::string_view filepath);
    const SubTexture* get_subtexture(const std::string& name);

    extern std::unordered_map<std::string, SubTexture> textures;
}
