#pragma once

#include "aporia_assets.hpp"
#include "aporia_string.hpp"
#include "aporia_types.hpp"

namespace Aporia
{
    struct Bitmap
    {
        u8* pixels = nullptr;
        i32 width = 0;
        i32 height = 0;
        i32 channels = 0;
    };

    Bitmap load_bitmap(MemoryArena* arena, String filepath);

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
        v2 u{ 0.f };
        v2 v{ 0.f };
        Texture* source = nullptr;
    };

    u32 get_next_texture_unit_for_id(u32 texture_id);
    void clear_texture_unit_map();

    // @NOTE(dubgron): Atlas refers to the config file describing the subtextures inside a texture.
    bool load_texture_atlas(String filepath);

    Texture* find_or_load_texture(String filepath);
    bool reload_texture_asset(Asset* texture_asset);

    const SubTexture* get_subtexture(String name);
    f32 get_subtexture_width(const SubTexture& subtexture);
    f32 get_subtexture_height(const SubTexture& subtexture);
}
