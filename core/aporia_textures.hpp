#pragma once

#include "platform/opengl_common.hpp"

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
        texture_id id = 0;

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
    const SubTexture& get_subtexture(const std::string& name);
}
