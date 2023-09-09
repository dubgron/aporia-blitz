#pragma once

#include <unordered_map>

#include "aporia_assets.hpp"
#include "aporia_strings.hpp"
#include "aporia_types.hpp"

namespace Aporia
{
    struct Image
    {
        u8* pixels = nullptr;

        i32 width = 0;
        i32 height = 0;
        i32 channels = 0;

        void load(String filepath);
        void unload();

        bool is_valid() const;
    };

    struct Texture
    {
        u32 id = 0;

        i32 width = 0;
        i32 height = 0;
        i32 channels = 0;

        String source_file;
    };

    struct SubTexture
    {
        String name;

        v2 u{ 0.f };
        v2 v{ 0.f };
        Texture* source = nullptr;
    };

    // @NOTE(dubgron): Atlas refers to the config file describing the subtextures inside a texture.
    bool load_texture_atlas(String filepath);

    Texture* find_or_load_texture(String filepath);
    bool reload_texture_asset(Asset* texture_asset);

    const SubTexture* get_subtexture(String name);
    f32 get_subtexture_width(const SubTexture& subtexture);
    f32 get_subtexture_height(const SubTexture& subtexture);
}
