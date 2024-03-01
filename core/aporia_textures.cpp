#include "aporia_textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include <stb_image.h>

#include "aporia_assets.hpp"
#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_hash_table.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    static constexpr u64 MAX_TEXTURES = 10;
    static constexpr u64 MAX_SUBTEXTURES = 512;

    // @NOTE(dubgron): The number of textures would be low, so we don't need to use
    // hash tables. Using a non-resizable array also gives us pointer stability.
    static Texture textures[MAX_TEXTURES];
    static u64 last_valid_texture_idx = 0;

    static HashTable<SubTexture> subtextures;

    Bitmap load_bitmap(MemoryArena* arena, String filepath)
    {
        Bitmap result;

        ScratchArena temp = get_scratch_arena(arena);
        {
            String contents = read_entire_file(temp.arena, filepath);
            result.pixels = stbi_load_from_memory(contents.data, contents.length, &result.width, &result.height, &result.channels, 4);

            // @HACK(dubgron): It would be nice if we didn't have to use malloc in stb_image
            // and have the bitmap already pushed on the arena.
            u64 num_of_pixels = result.width * result.height * result.channels;
            u8* pixels = arena->push<u8>(num_of_pixels);
            memcpy(pixels, result.pixels, num_of_pixels);

            stbi_image_free(result.pixels);
            result.pixels = pixels;
        }
        release_scratch_arena(temp);

        if (result.pixels)
        {
            APORIA_LOG(Info, "Bitmap '%' loaded successfully!", filepath);
        }
        else
        {
            APORIA_LOG(Error, "Failed to load image '%'!", filepath);
        }

        return result;
    }

    // @TODO(dubgron): This will fail if we reload textures too many times. Make it more robust.
    u32 get_next_texture_unit()
    {
        static u32 unit = 0;
        return unit++;
    }

    static bool operator==(const SubTexture& subtexture1, const SubTexture& subtexture2)
    {
        return subtexture1.u == subtexture2.u && subtexture1.v == subtexture2.v && subtexture1.source == subtexture2.source;
    }

    bool load_texture_atlas(String filepath)
    {
        ScratchArena temp = get_scratch_arena();
        Config_Property* parsed_file = parse_config_from_file(temp.arena, filepath);

        String texture_filepath;
        for (Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category == "meta" && property->field == "filepath")
            {
                texture_filepath = property->literals.first->string;
                break;
            }
        }

        if (texture_filepath.length == 0)
        {
            APORIA_LOG(Error, "Failed to get [meta.filepath] property from %s", filepath);
            return false;
        }

        Texture* atlas_texture = find_or_load_texture(texture_filepath);

        if (!hash_table_is_created(&subtextures))
        {
            subtextures = hash_table_create<SubTexture>(&memory.persistent, MAX_SUBTEXTURES);
        }

        for (Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category != "subtextures")
            {
                continue;
            }

            String name = push_string(&memory.persistent, property->field);

            if (hash_table_find(&subtextures, name) != nullptr)
            {
                APORIA_LOG(Warning, "There is more than one subtexture named '%'! One of them will be overwritten!", name);
            }

            const v2 u{ string_to_float(property->inner->literals.first->string), string_to_float(property->inner->literals.last->string) };
            const v2 v{ string_to_float(property->inner->next->literals.first->string), string_to_float(property->inner->next->literals.last->string) };

            SubTexture subtexture;
            subtexture.u = u;
            subtexture.v = v;
            subtexture.source = atlas_texture;
            hash_table_insert(&subtextures, name, subtexture);

            APORIA_ASSERT(*hash_table_find(&subtextures, name) == subtexture);
        }

        APORIA_LOG(Info, "All textures from '%' loaded successfully", filepath);

        release_scratch_arena(temp);

        return true;
    }

    static Texture* add_texture(const Texture& texture)
    {
        Texture* found_spot = nullptr;
        for (u64 idx = 0; idx < last_valid_texture_idx; ++idx)
        {
            if (textures[idx].id == 0)
            {
                found_spot = &textures[idx];
                break;
            }
        }

        if (!found_spot)
        {
            APORIA_ASSERT(last_valid_texture_idx < MAX_TEXTURES);
            found_spot = &textures[last_valid_texture_idx];
            last_valid_texture_idx += 1;
        }

        APORIA_ASSERT(found_spot);
        *found_spot = texture;

        return found_spot;
    }

    static Texture* load_texture_from_file(String filepath)
    {
        ScratchArena temp = get_scratch_arena();

        Bitmap bitmap = load_bitmap(temp.arena, filepath);
        if (!bitmap.pixels)
        {
            release_scratch_arena(temp);
            return nullptr;
        }

        u32 id = 0;
        u32 unit = get_next_texture_unit();

#if defined(APORIA_EMSCRIPTEN)
        glGenTextures(1, &id);

        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, bitmap.width, bitmap.height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bitmap.width, bitmap.height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenerateMipmap(GL_TEXTURE_2D);
#else
        glCreateTextures(GL_TEXTURE_2D, 1, &id);

        glBindTextureUnit(unit, id);

        glTextureStorage2D(id, 1, GL_RGBA8, bitmap.width, bitmap.height);
        glTextureSubImage2D(id, 0, 0, 0, bitmap.width, bitmap.height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.pixels);

        // @TODO(dubgron): Texture parameters should be parameterizable.
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenerateTextureMipmap(id);
#endif

        release_scratch_arena(temp);

        if (!id)
        {
            APORIA_LOG(Error, "Failed to create OpenGL texture from '%'!", filepath);
            return nullptr;
        }

        Texture texture;
        texture.id = id;
        texture.unit = unit;
        texture.width = bitmap.width;
        texture.height = bitmap.height;
        texture.channels = bitmap.channels;
        // @NOTE(dubgron): The texture doesn't take an ownership over the filepath.
        // We have to make sure the texture doesn't outlive it.
        texture.source_file = filepath;

        return add_texture(texture);
    }

    Texture* find_or_load_texture(String filepath)
    {
        for (i64 idx = 0; idx < last_valid_texture_idx; ++idx)
        {
            // Texture already loaded.
            if (textures[idx].source_file == filepath)
            {
                return &textures[idx];
            }
        }

        // Failed to find the texture, we need to load it.
        Asset* texture_asset = register_asset(filepath, AssetType::Texture);

        Texture* result = load_texture_from_file(texture_asset->source_file);
        texture_asset->status = result ? AssetStatus::Loaded : AssetStatus::NotLoaded;

        return result;
    }

    bool reload_texture_asset(Asset* texture_asset)
    {
        APORIA_ASSERT(texture_asset->type == AssetType::Texture);

        for (i64 idx = 0; idx < last_valid_texture_idx; ++idx)
        {
            Texture* texture = &textures[idx];
            if (texture->source_file == texture_asset->source_file)
            {
                glDeleteTextures(1, &texture->id);
                texture->id = 0;

                // @NOTE(dubgron): This should reload the new texture into the
                // same spot as an old one because its ID has been zeroed out.
                Texture* reloaded_texture = load_texture_from_file(texture_asset->source_file);
                texture_asset->status = reloaded_texture ? AssetStatus::Loaded : AssetStatus::NotLoaded;

                return reloaded_texture != nullptr;
            }
        }

        APORIA_LOG(Warning, "Failed to find texture with source file: '%'!", texture_asset->source_file);
        return false;
    }

    const SubTexture* get_subtexture(String name)
    {
        SubTexture* subtexture = hash_table_find(&subtextures, name);
        if (!subtexture)
        {
            APORIA_LOG(Error, "Failed to find subtexture '%'!", name);
        }
        return subtexture;
    }

    f32 get_subtexture_width(const SubTexture& subtexture)
    {
        return subtexture.source ? (subtexture.v.x - subtexture.u.x) * subtexture.source->width : 0.f;
    }

    f32 get_subtexture_height(const SubTexture& subtexture)
    {
        return subtexture.source ? (subtexture.v.y - subtexture.u.y) * subtexture.source->height : 0.f;
    }
}
