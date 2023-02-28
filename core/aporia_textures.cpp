#include "aporia_textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "utils/read_file.hpp"

namespace Aporia
{
    std::unordered_map<std::string, SubTexture> textures;

    void Image::load(std::string_view filepath)
    {
        if (pixels)
        {
            APORIA_LOG(Info, "Image '{}' has already been loaded! Unloading first!", filepath);
            unload();
        }

        pixels = stbi_load(filepath.data(), &width, &height, &channels, 4);
        APORIA_LOG(Info, "Image '{}' loaded successfully!", filepath);
    }

    void Image::unload()
    {
        stbi_image_free(pixels);
        pixels = nullptr;
    }

    void load_texture_atlas(std::string_view filepath)
    {
        APORIA_VALIDATE_OR_RETURN(std::filesystem::exists(filepath), "File '{}' does not open!", filepath)

        std::string data = read_file(filepath);

        using json = nlohmann::json;
        json texture_json = json::parse(data);

        const std::string atlas_filepath = texture_json["atlas"];

        APORIA_VALIDATE_OR_RETURN(std::filesystem::exists(atlas_filepath), "File '{}' does not open!", filepath)

        static u32 id = 0;
        glDeleteTextures(1, &id);

        Image atlas_image;
        atlas_image.load(atlas_filepath);

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

        const Texture atlas_texture{ static_cast<texture_id>(id), atlas_image.width, atlas_image.height, atlas_image.channels };
        for (auto& texture : texture_json["textures"])
        {
            std::string name = texture["name"];

            if (textures.contains(name))
            {
                APORIA_LOG(Warning, "There are two textures named '{}'! One of them will be overwritten!", name);
            }

            const v2 u{ texture["u"][0], texture["u"][1] };
            const v2 v{ texture["v"][0], texture["v"][1] };

            textures.try_emplace(name, SubTexture{ u, v, atlas_texture });
        }

        APORIA_LOG(Info, "All textures from '{}' loaded successfully", filepath);
    }

    const SubTexture& get_subtexture(const std::string& name)
    {
        static const SubTexture EmptySubTexture;
        APORIA_VALIDATE_OR_RETURN_VALUE(textures.contains(name), EmptySubTexture,
            "Failed to find sub texture '{}'! Returned default texture!", name);

        return textures.find(name)->second;
    }
}
