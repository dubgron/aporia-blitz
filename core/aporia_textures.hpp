#pragma once

#include "aporia_assets.hpp"
#include "aporia_string.hpp"
#include "aporia_types.hpp"

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
    i64 texture_index = INDEX_INVALID;
};

// @NOTE(dubgron): Atlas refers to the config file describing the subtextures inside a texture.
bool load_texture_atlas(String filepath);

i64 find_or_load_texture_index(String filepath);
bool reload_texture_asset(Asset* texture_asset);

Texture* get_texture(i64 index);
SubTexture* get_subtexture(String name);
void get_subtexture_size(const SubTexture& subtexture, f32* width, f32* height);
String get_subtexture_name(const SubTexture& subtexture);
