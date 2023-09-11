#include "aporia_textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
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

    // @NOTE(dubgron): The number of textures would be low, so we don'tneed to use
    // hash tables. Using a non-resizable array also gives us pointer stability.
    static Texture textures[MAX_TEXTURES];
    static u64 last_valid_texture_idx = 0;

    static HashTable<SubTexture> subtextures;

    // @TODO(dubgron): Make it into free functions, it will be more convenient to use.
    void Image::load(String filepath)
    {
        if (pixels)
        {
            APORIA_LOG(Warning, "Image '%' has already been loaded! Unloading first!", filepath);
            unload();
        }

        pixels = stbi_load(*filepath, &width, &height, &channels, 4);

        if (pixels)
        {
            APORIA_LOG(Info, "Image '%' loaded successfully!", filepath);
        }
        else
        {
            APORIA_LOG(Error, "Failed to load image '%'!", filepath);
        }
    }

    void Image::unload()
    {
        stbi_image_free(pixels);
        pixels = nullptr;
    }

    bool Image::is_valid() const
    {
        return pixels != nullptr;
    }

    static bool operator==(const SubTexture& subtexture1, const SubTexture& subtexture2)
    {
        return subtexture1.u == subtexture2.u && subtexture1.v == subtexture2.v && subtexture1.source == subtexture2.source;
    }

    bool load_texture_atlas(String filepath)
    {
        ScratchArena temp = create_scratch_arena(&frame_arena);
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
            subtextures = hash_table_create<SubTexture>(&persistent_arena, MAX_SUBTEXTURES);
        }

        for (Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category != "subtextures")
            {
                continue;
            }

            String name = push_string(&persistent_arena, property->field);

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

        rollback_scratch_arena(temp);

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
        Image image;
        image.load(filepath);
        if (!image.is_valid())
        {
            return nullptr;
        }

        u32 id = 0;

#if defined(APORIA_EMSCRIPTEN)
        glGenTextures(1, &id);

        glActiveTexture(GL_TEXTURE0 + id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, image.width, image.height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenerateMipmap(GL_TEXTURE_2D);
#else
        glCreateTextures(GL_TEXTURE_2D, 1, &id);

        glBindTextureUnit(id, id);

        glTextureStorage2D(id, 1, GL_RGBA8, image.width, image.height);
        glTextureSubImage2D(id, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);

        // @TODO(dubgron): Texture parameters should be parameterizable.
        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenerateTextureMipmap(id);
#endif

        image.unload();

        if (!id)
        {
            APORIA_LOG(Error, "Failed to create OpenGL texture from '%'!", filepath);
            return nullptr;
        }

        Texture texture;
        texture.id = id;
        texture.width = image.width;
        texture.height = image.height;
        texture.channels = image.channels;
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
        Texture* texture = load_texture_from_file(filepath);

        Asset* texture_asset = register_asset(filepath, AssetType::Texture);
        texture_asset->status = texture ? AssetStatus::Loaded : AssetStatus::NotLoaded;

        return texture;
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

                bool reloaded_successfully = load_texture_from_file(texture_asset->source_file) != nullptr;
                texture_asset->status = reloaded_successfully ? AssetStatus::Loaded : AssetStatus::NotLoaded;

                return reloaded_successfully;
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
