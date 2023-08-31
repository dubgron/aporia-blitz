#include "aporia_textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    std::unordered_map<String, SubTexture> textures;

    void Image::load(String filepath)
    {
        if (pixels)
        {
            APORIA_LOG(Info, "Image '%' has already been loaded! Unloading first!", filepath);
            unload();
        }

        pixels = stbi_load(*filepath, &width, &height, &channels, 4);

        if (pixels)
        {
            APORIA_LOG(Info, "Image '%' loaded successfully!", filepath);
        }
        else
        {
            APORIA_LOG(Info, "Failed to load image '%'!", filepath);
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

    void load_texture_atlas(String filepath)
    {
        ScratchArena temp = create_scratch_arena(&frame_arena);
        Config_Property* parsed_file = parse_config_from_file(temp.arena, filepath);

        String atlas_filepath;
        for (Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category == "meta" && property->field == "filepath")
            {
                atlas_filepath = property->literals.first->string;
                break;
            }
        }

        if (atlas_filepath.length == 0)
        {
            APORIA_LOG(Error, "Failed to get [meta.filepath] property from %s", filepath);
            return;
        }

        u32 id = 0;
        glDeleteTextures(1, &id);

        Image atlas_image;
        atlas_image.load(atlas_filepath);
        APORIA_ASSERT(atlas_image.is_valid());

#if defined(APORIA_EMSCRIPTEN)
        glGenTextures(1, &id);

        glActiveTexture(GL_TEXTURE0 + id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, atlas_image.width, atlas_image.height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, atlas_image.width, atlas_image.height, GL_RGBA, GL_UNSIGNED_BYTE, atlas_image.pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenerateMipmap(GL_TEXTURE_2D);
#else
        glCreateTextures(GL_TEXTURE_2D, 1, &id);

        glBindTextureUnit(id, id);

        glTextureStorage2D(id, 1, GL_RGBA8, atlas_image.width, atlas_image.height);
        glTextureSubImage2D(id, 0, 0, 0, atlas_image.width, atlas_image.height, GL_RGBA, GL_UNSIGNED_BYTE, atlas_image.pixels);

        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenerateTextureMipmap(id);
#endif

        const Texture atlas_texture{ id, atlas_image.width, atlas_image.height, atlas_image.channels };
        for (Config_Property* property = parsed_file; property; property = property->next)
        {
            if (property->category != "subtextures")
            {
                continue;
            }

            String name = push_string(&persistent_arena, property->field);
            if (textures.contains(name))
            {
                APORIA_LOG(Warning, "There are two textures named '%'! One of them will be overwritten!", name);
            }

            const v2 u{ string_to_float(property->inner->literals.first->string), string_to_float(property->inner->literals.last->string) };
            const v2 v{ string_to_float(property->inner->next->literals.first->string), string_to_float(property->inner->next->literals.last->string) };

            textures.try_emplace(name, SubTexture{ u, v, atlas_texture });
        }

        APORIA_LOG(Info, "All textures from '%' loaded successfully", filepath);

        atlas_image.unload();
    }

    const SubTexture* get_subtexture(String name)
    {
        APORIA_ASSERT_WITH_MESSAGE(textures.contains(name),
            "Failed to find sub texture '%'!", name);

        return &textures.find(name)->second;
    }

    f32 get_subtexture_width(const SubTexture& subtexture)
    {
        return (subtexture.v.x - subtexture.u.x) * subtexture.source.width;
    }

    f32 get_subtexture_height(const SubTexture& subtexture)
    {
        return (subtexture.v.y - subtexture.u.y) * subtexture.source.height;
    }
}
