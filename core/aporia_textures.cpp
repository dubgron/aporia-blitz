#include "aporia_textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include <stb_image.h>

#include "aporia_assets.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_hash_table.hpp"
#include "aporia_parser.hpp"
#include "aporia_utils.hpp"

static constexpr u64 MAX_TEXTURES = 32;
static constexpr u64 MAX_SUBTEXTURES = 2048;

// @NOTE(dubgron): The number of textures would be low, so we don't need to use
// hash tables. Using a non-resizable array also gives us pointer stability.
static Texture textures[MAX_TEXTURES];
static u64 last_valid_texture_idx = 0;

static HashTable<SubTexture> subtextures;

Bitmap load_bitmap(MemoryArena* arena, String filepath)
{
    Bitmap result;

    ScratchArena temp = scratch_begin(arena);
    {
        String contents = read_entire_file(temp.arena, filepath);
        result.pixels = stbi_load_from_memory(contents.data, contents.length, &result.width, &result.height, &result.channels, 0);

        // @HACK(dubgron): It would be nice if we didn't have to use malloc in stb_image
        // and have the bitmap already pushed onto the arena.
        u64 num_of_pixels = result.width * result.height * result.channels;
        u8* pixels = arena_push_uninitialized<u8>(arena, num_of_pixels);
        memcpy(pixels, result.pixels, num_of_pixels);

        stbi_image_free(result.pixels);
        result.pixels = pixels;
    }
    scratch_end(temp);

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

static bool operator==(const SubTexture& subtexture1, const SubTexture& subtexture2)
{
    return subtexture1.u == subtexture2.u && subtexture1.v == subtexture2.v && subtexture1.texture_index == subtexture2.texture_index;
}

i64 load_texture_atlas(String filepath)
{
    ScratchArena temp = scratch_begin();
    ParseTreeNode* parsed_file = parse_from_file(temp.arena, filepath);

    ParseTreeNode* subtextures_node = nullptr;

    String texture_filepath;
    for (ParseTreeNode* node = parsed_file->child_first; node; node = node->next)
    {
        APORIA_ASSERT(node->type == ParseTreeNode_Category);
        if (node->name == "meta")
        {
            for (ParseTreeNode* meta = node->child_first; meta; meta = meta->next)
            {
                if (meta->name == "filepath")
                {
                    get_value_from_field(meta, &texture_filepath);
                    break;
                }
            }
        }

        if (node->name == "subtextures")
        {
            subtextures_node = node;
        }
    }

    if (texture_filepath.length == 0)
    {
        APORIA_LOG(Error, "Failed to find [meta.filepath] property in '%'", filepath);
        return INDEX_INVALID;
    }

    if (!subtextures_node)
    {
        APORIA_LOG(Error, "Failed to find [subtextures] category in '%'", filepath);
        return INDEX_INVALID;
    }

    i64 atlas_texture = find_or_load_texture_index(texture_filepath);

    if (!hash_table_is_created(&subtextures))
    {
        subtextures = hash_table_create<SubTexture>(&memory.persistent, MAX_SUBTEXTURES);
    }

    for (ParseTreeNode* subtexture_node = subtextures_node->child_first; subtexture_node; subtexture_node = subtexture_node->next)
    {
        APORIA_ASSERT(subtexture_node->type == ParseTreeNode_Struct && subtexture_node->child_count == 2);
        String name = push_string(&memory.persistent, subtexture_node->name);

        if (hash_table_find(&subtextures, name) != nullptr)
        {
            APORIA_LOG(Warning, "There is more than one subtexture named '%'! One of them will be overwritten!", name);
        }

        ParseTreeNode* u_node = subtexture_node->child_first;
        ParseTreeNode* v_node = subtexture_node->child_last;

        v2 u, v;
        get_value_from_field(u_node, &u[0], 2);
        get_value_from_field(v_node, &v[0], 2);

        SubTexture subtexture;
        subtexture.u = u;
        subtexture.v = v;
        subtexture.texture_index = atlas_texture;
        hash_table_insert(&subtextures, name, subtexture);

        APORIA_ASSERT(*hash_table_find(&subtextures, name) == subtexture);
    }

    APORIA_LOG(Info, "All textures from '%' loaded successfully", filepath);

    scratch_end(temp);

    return atlas_texture;
}

static i64 add_texture(const Texture& texture)
{
    i64 found_spot = INDEX_INVALID;
    for (u64 idx = 0; idx < last_valid_texture_idx; ++idx)
    {
        if (textures[idx].id == 0)
        {
            found_spot = idx;
            break;
        }
    }

    if (found_spot == INDEX_INVALID)
    {
        APORIA_ASSERT(last_valid_texture_idx < MAX_TEXTURES);
        found_spot = last_valid_texture_idx;
        last_valid_texture_idx += 1;
    }

    APORIA_ASSERT(found_spot != INDEX_INVALID);
    textures[found_spot] = texture;

    return found_spot;
}

static i64 load_texture_from_file(String filepath)
{
    ScratchArena temp = scratch_begin();

    Bitmap bitmap = load_bitmap(temp.arena, filepath);
    if (!bitmap.pixels)
    {
        scratch_end(temp);
        return INDEX_INVALID;
    }

    u32 sized_format, base_format;
    switch (bitmap.channels)
    {
        case 1: sized_format = GL_R8;       base_format = GL_RED;   break;
        case 2: sized_format = GL_RG8;      base_format = GL_RG;    break;
        case 3: sized_format = GL_RGB8;     base_format = GL_RGB;   break;
        case 4: sized_format = GL_RGBA8;    base_format = GL_RGBA;  break;
        default: APORIA_UNREACHABLE();
    }

    // @NOTE(dubgron): If the bitmap has an uneven number of channels, we set its
    // alignment to 1 to guarantee the pixel rows are contiguous in memory.
    // See https://www.khronos.org/opengl/wiki/Pixel_Transfer#Pixel_layout.
    if (bitmap.channels % 2 == 1)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    u32 id = 0;

#if defined(APORIA_EMSCRIPTEN)
    glGenTextures(1, &id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexStorage2D(GL_TEXTURE_2D, 1, sized_format, bitmap.width, bitmap.height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bitmap.width, bitmap.height, base_format, GL_UNSIGNED_BYTE, bitmap.pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D);
#else
    glCreateTextures(GL_TEXTURE_2D, 1, &id);

    glBindTextureUnit(0, id);

    glTextureStorage2D(id, 1, sized_format, bitmap.width, bitmap.height);
    glTextureSubImage2D(id, 0, 0, 0, bitmap.width, bitmap.height, base_format, GL_UNSIGNED_BYTE, bitmap.pixels);

    // @TODO(dubgron): Texture parameters should be parameterizable.
    glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateTextureMipmap(id);
#endif

    scratch_end(temp);

    if (!id)
    {
        APORIA_LOG(Error, "Failed to create OpenGL texture from '%'!", filepath);
        return INDEX_INVALID;
    }

    Texture texture;
    texture.id = id;
    texture.width = bitmap.width;
    texture.height = bitmap.height;
    texture.channels = bitmap.channels;
    // @NOTE(dubgron): The texture doesn't take an ownership over the filepath.
    // We have to make sure the texture doesn't outlive it.
    texture.source_file = filepath;

    return add_texture(texture);
}

i64 find_or_load_texture_index(String filepath)
{
    for (i64 idx = 0; idx < last_valid_texture_idx; ++idx)
    {
        // Texture already loaded.
        if (textures[idx].source_file == filepath)
        {
            return idx;
        }
    }

    // Failed to find the texture, we need to load it.
    Asset* texture_asset = register_asset(filepath, AssetType::Texture);

    i64 result = load_texture_from_file(texture_asset->source_file);
    texture_asset->status = result != INDEX_INVALID ? AssetStatus::Loaded : AssetStatus::NotLoaded;

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
            i64 reloaded_texture = load_texture_from_file(texture_asset->source_file);
            texture_asset->status = reloaded_texture != INDEX_INVALID ? AssetStatus::Loaded : AssetStatus::NotLoaded;

            return reloaded_texture != INDEX_INVALID;
        }
    }

    APORIA_LOG(Warning, "Failed to find texture with source file: '%'!", texture_asset->source_file);
    return false;
}

Texture* get_texture(i64 index)
{
    if (index != INDEX_INVALID && index < last_valid_texture_idx)
    {
        return &textures[index];
    }

    return nullptr;
}

SubTexture* get_subtexture(String name)
{
    SubTexture* subtexture = hash_table_find(&subtextures, name);
    if (!subtexture)
    {
        APORIA_LOG(Error, "Failed to find subtexture '%'!", name);
    }
    return subtexture;
}

void get_subtexture_size(const SubTexture& subtexture, f32* width, f32* height)
{
    if (Texture* texture = get_texture(subtexture.texture_index))
    {
        *width = (subtexture.v.x - subtexture.u.x) * texture->width;
        *height = (subtexture.v.y - subtexture.u.y) * texture->height;
    }
}

String get_subtexture_name(const SubTexture& subtexture)
{
    for (i64 idx = 0; idx < subtextures.bucket_count; ++idx)
    {
        if (subtexture == subtextures.buckets[idx].value)
        {
            return subtextures.buckets[idx].key;
        }
    }

    APORIA_LOG(Warning, "Failed to find subtexture 'u: (%, %), v: (%, %), texture_index: %'!",
        subtexture.u.x, subtexture.u.y, subtexture.v.x, subtexture.v.y, subtexture.texture_index);
    return String{};
}
