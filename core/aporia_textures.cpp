#include "aporia_textures.hpp"

static MemoryArena* stbi_arena = nullptr;

static void* stbi_realloc(void* ptr, u64 new_size)
{
    void* result = arena_push_uninitialized(stbi_arena, new_size);
    return ptr ? ptr : result;
}

#define STBI_MALLOC(size)           arena_push_uninitialized(stbi_arena, size)
#define STBI_REALLOC(ptr, new_size) stbi_realloc(ptr, new_size)
#define STBI_FREE(ptr)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include <stb_image.h>

#include <zlib.h>

#include "aporia_assets.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_parser.hpp"
#include "aporia_utils.hpp"

static constexpr u64 MAX_TEXTURES = 32;
static constexpr u64 MAX_SUBTEXTURES = 2048;

// @NOTE(dubgron): The number of textures would be low, so we don't need to use
// hash tables. Using a non-resizable array also gives us pointer stability.
static Texture textures[MAX_TEXTURES];
static u64 last_valid_texture_idx = 0;

HashTable<SubTexture> subtextures;

Bitmap load_bitmap(MemoryArena* arena, String filepath)
{
    Bitmap result;

    ScratchArena temp = scratch_begin(arena);
    stbi_arena = temp.arena;
    {
        String contents = read_entire_file(temp.arena, filepath);
        result.pixels = stbi_load_from_memory(contents.data, contents.length, &result.width, &result.height, &result.channels, 0);

        u64 num_of_pixels = result.width * result.height * result.channels;
        u8* pixels = arena_push_uninitialized<u8>(arena, num_of_pixels);
        memcpy(pixels, result.pixels, num_of_pixels);

        result.pixels = pixels;
    }
    stbi_arena = nullptr;
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

bool operator==(const SubTexture& tex0, const SubTexture& tex1)
{
    return tex0.u == tex1.u && tex0.v == tex1.v && tex0.texture_index == tex1.texture_index;
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

static i64 load_texture_from_bitmap(Bitmap bitmap)
{
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

    Texture texture;
    texture.id = id;
    texture.width = bitmap.width;
    texture.height = bitmap.height;
    texture.channels = bitmap.channels;

    i64 texture_index = add_texture(texture);
    return texture_index;
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

    i64 texture_index = load_texture_from_bitmap(bitmap);

    // @NOTE(dubgron): The texture doesn't take an ownership over the filepath.
    // We have to make sure the texture doesn't outlive it.
    textures[texture_index].source_file = filepath;

    if (texture_index == INDEX_INVALID)
    {
        APORIA_LOG(Error, "Failed to create OpenGL texture from '%'!", filepath);
    }

    scratch_end(temp);

    return texture_index;
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
    if (subtexture.texture_index == INDEX_INVALID)
        return "INVALID";

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

//////////////////////////////////////////////////
// Aseprite

#pragma pack(push, 1)

struct AsepriteOldPaletteChunkData
{
    struct Color
    {
        u8 red = 0;
        u8 green  = 0;
        u8 blue = 0;
    };

    struct Packet
    {
        u8 palette_entries_to_skip_from_last_packet_count = 0;  // start from 0
        u8 color_count = 0;                                     // 0 means 256
        Color* colors = nullptr;
    };

    u16 packet_count = 0;
    Packet* packets = nullptr;
};

using AsepriteLayerFlags = u16;
enum AsepriteLayerFlag_ : AsepriteLayerFlags
{
    AsepriteLayerFlag_Visible                           = 0x01,
    AsepriteLayerFlag_Editable                          = 0x02,
    AsepriteLayerFlag_LockMovement                      = 0x04,
    AsepriteLayerFlag_Background                        = 0x08,
    AsepriteLayerFlag_PreferLinkedCels                  = 0x10,
    AsepriteLayerFlag_GroupShouldBeDisplayedCollapsed   = 0x20,
    AsepriteLayerFlag_ReferenceLayer                    = 0x30,
};

enum AsepriteLayerType : u16
{
    AsepriteLayer_Normal    = 0,
    AsepriteLayer_Group     = 1,
    AsepriteLayer_Tilemap   = 2,
};

enum AsepriteBlendMode : u16
{
    AsepriteBlendMode_Normal        = 0,
    AsepriteBlendMode_Multiply      = 1,
    AsepriteBlendMode_Screen        = 2,
    AsepriteBlendMode_Overlay       = 3,
    AsepriteBlendMode_Darken        = 4,
    AsepriteBlendMode_Lighten       = 5,
    AsepriteBlendMode_ColorDodge    = 6,
    AsepriteBlendMode_ColorBurn     = 7,
    AsepriteBlendMode_HardLight     = 8,
    AsepriteBlendMode_SoftLight     = 9,
    AsepriteBlendMode_Difference    = 10,
    AsepriteBlendMode_Exclusion     = 11,
    AsepriteBlendMode_Hue           = 12,
    AsepriteBlendMode_Saturation    = 13,
    AsepriteBlendMode_Color         = 14,
    AsepriteBlendMode_Luminosity    = 15,
    AsepriteBlendMode_Addition      = 16,
    AsepriteBlendMode_Subtract      = 17,
    AsepriteBlendMode_Divide        = 18,
};

struct AsepriteString
{
    u16 count = 0;
    u8* data = nullptr;
};

struct AsepriteLayerChunkData
{
    struct Header
    {
        AsepriteLayerFlags flags = 0;
        AsepriteLayerType type = AsepriteLayer_Normal;
        u16 child_level = 0;
        u16 default_layer_width_in_pixels = 0;                      // ignored
        u16 default_layer_height_in_pixels = 0;                     // ignored
        AsepriteBlendMode blend_mode = AsepriteBlendMode_Normal;    // always 0 for layer set
        u8 opacity = 0;                                             // valid only if file header flags field has bit 1 set
        u8 _unused[3] = { 0 };                                      // for future (set to zero)
    } header;

    AsepriteString layer_name;
    u32 tileset_index = 0; // if layer type = 2
};

enum AsepriteCelType : u16
{
    AsepriteCel_RawImageData        = 0, // unused, compressed image is preferred
    AsepriteCel_LinkedCel           = 1,
    AsepriteCel_CompressedImage     = 2,
    AsepriteCel_CompressedTilemap   = 3,
};

struct AsepriteCelChunkData
{
    struct Header
    {
        u16 layer_index = 0;
        i16 x_position = 0;
        i16 y_position = 0;
        u8 opacity_level = 0;
        AsepriteCelType cel_type = (AsepriteCelType)0;
        i16 z_index = 0;        // 0 = default layer ordering; +N = show this cel N layers later; -N = show this cel N layers back
        u8 _unused[5] = { 0 };  // for future (set to zero)
    } header;

    struct RawImageData // for cel type = 0 (Raw Image Data)
    {
        u16 width_in_pixels = 0;
        u16 height_in_pixels = 0;
        u8* raw_pixels = nullptr; // row by row from top to bottom, for each scanline read pixels from left to right
    };

    struct LinkedCel // for cel type = 1 (Linked Cel)
    {
        u16 frame_position_to_link_with = 0;
    };

    struct CompressedImage // for cel type = 2 (Compressed Image)
    {
        u16 width_in_pixels = 0;
        u16 height_in_pixels = 0;
        u8* raw_cel = nullptr; // compressed with ZLIB method (in .aseprite file)
    };

    struct CompressedTilemap // for cel type = 3 (Compressed Tilemap)
    {
        struct Header
        {
            u16 width_in_tiles = 0;
            u16 height_in_tiles = 0;
            u16 bits_per_tile = 0;          // at the moment it's always 32-bit per tile
            u32 bitmask_tile_id = 0;        // e.g. 0x1fffffff for 32-bit tiles
            u32 bitmask_x_flip = 0;
            u32 bitmask_y_flip = 0;
            u32 bitmask_diagonal_flip = 0;  // swap X/Y axis
            u8 _unused[10] = { 0 };         // reserved
        } header;

        u8* tiles = nullptr; // Row by row, from top to bottom tile by tile compressed with ZLIB method (in .aseprite file)
    };

    union
    {
        RawImageData raw_image_data;
        LinkedCel linked_cel;
        CompressedImage compressed_image;
        CompressedTilemap compressed_tilemap;
    };
};

struct AsepriteFixed
{
    i32 value = 0;

    operator f32() const
    {
        constexpr f32 a = 1.f / (1 << 16);
        return (f32)value * a;
    }
};

struct AsepriteCelExtraChunkData
{
    u32 flags = 0;                              // 1 = precise bounds are set
    AsepriteFixed precise_x_position{ 0 };
    AsepriteFixed precise_y_position{ 0 };
    AsepriteFixed width_of_cel_in_sprite{ 0 };  // scaled in real-time
    AsepriteFixed height_of_cel_in_sprite{ 0 };
    u8 _unused[16] = { 0 };                     // for future use (set to zero)
};

struct AsepriteColorProfileChunkData
{
    struct Header
    {
        u16 type = 0;                   // 0 - no color profile (as in old .aseprite files), 1 - use sRGB, 2 - use the embedded ICC profile
        u16 flags = 0;                  // 1 - use special fixed gamma
        AsepriteFixed fixed_gamma{ 0 }; // 1.0 = linear, Note: The gamma in sRGB is 2.2 in overall but it doesn't use this fixed gamma, because sRGB uses different gamma sections (linear and non-linear). If sRGB is specified with a fixed gamma = 1.0, it means that this is Linear sRGB.
        u8 _unused[8] = { 0 };          // reserved (set to zero)
    } header;

    u32 icc_profile_data_length = 0;
    u8* icc_profile_data = nullptr;
};

struct AsepriteExternalFilesChunkData
{
    struct Entry
    {
        struct Header
        {
            u32 entry_id = 0;       // this ID is referenced by tilesets, palettes, or extended properties
            u8 type = 0;            // 0 - external palette, 1 - external tileset, 2 - extension name for properties, 3 - extension name for tile management (can exist one per sprite)
            u8 _unused[7] = { 0 };  // reserved (set to zero)
        } header;

        AsepriteString external_file_name_or_extension_id;
    };

    struct Header
    {
        u32 entry_count = 0;
        u8 _unused[8] = { 0 }; // reserved (set to zero)
    } header;

    Entry* entries = nullptr;
};

struct AsepriteMaskChunkData // DEPRECATED
{
    struct Header
    {
        i16 x_position = 0;
        i16 y_position = 0;
        u32 width = 0;
        u32 height = 0;
        u8 _unused[8] = { 0 };
    } header;

    AsepriteString name;
    u8* bit_map_data = nullptr; // size = height*((width+7)/8); each byte contains 8 pixels (the leftmost pixels are packed into the high order bits)
};

struct AsepritePathChunkData // never used
{
};

struct AsepriteTagsChunkData
{
    struct Tag
    {
        struct Header
        {
            u16 from_frame = 0;
            u16 to_frame = 0;
            u8 loop_animation_direction = 0;    // 0 = forward, 1 = reverse, 2 = ping-pong, 3 = ping-pong reverse
            u16 repeat_count = 0;               // play this animation section N times: 0 = doesn't specify (plays infinite in UI, once on export, for ping-pong it plays once in each direction); 1 = plays once (for ping-pong, it plays just in one direction); 2 = plays twice (for ping-pong, it plays once in one direction, and once in reverse); n = plays N times
            u8 _unused1[6] = { 0 };             // for future (set to zero)
            u8 rgb_values[3] = { 0 };           // DEPRECATED, used only for backward compatibility with Aseprite v1.2.x; the color of the tag is the one in the user data field following the tags chunk
            u8 _unused2 = 0;
        } header;

        AsepriteString name;
    };

    struct Header
    {
        u16 tag_count = 0;
        u8 _unused[8] = {0}; // for future (set to zero)
    } header;

    Tag* tags = nullptr;
};

struct AsepriteColor
{
    u8 red = 0;
    u8 green = 0;
    u8 blue = 0;
    u8 alpha = 0;
};

struct AsepritePaletteChunkData
{
    struct PaletteEntry
    {
        struct Header
        {
            u16 flags = 0; // 1 = has name
            AsepriteColor color;
        } header;

        AsepriteString color_name;
    };

    struct Header
    {
        u32 new_palette_size = 0;   // total number of entries
        u32 first_color_index_to_change = 0;
        u32 last_color_index_to_change = 0;
        u8 _unused[8] = { 0 };      // for future (set_to_zero)
    } header;

    PaletteEntry* palette_entries = nullptr;
};

using AsepriteUserDataFlags = u32;
enum AsepriteUserDataFlag_ : AsepriteUserDataFlags
{
    AsepriteUserDataFlag_HasText        = 0x01,
    AsepriteUserDataFlag_HasColor       = 0x02,
    AsepriteUserDataFlag_HasProperties  = 0x04,
};

struct AsepritePoint
{
    i32 x_coordinate = 0;
    i32 y_coordiante = 0;
};

struct AsepriteSize
{
    i32 width = 0;
    i32 height = 0;
};

struct AsepriteRect
{
    AsepritePoint origin;
    AsepriteSize size;
};

struct AsepriteArray
{
    u32 count = 0;
    u16 type = 0;

    u16* types = nullptr;   // if element's type == 0 (all elements are not of the same type)
    u8* elements = nullptr; // else (all elements are of the same type)
};

struct AsepritePropertiesMap
{
    u32 key = 0; // == 0 means user properties, != 0 means an extension Entry ID (see External Files Chunk)
    u32 property_count = 0;
    struct AsepriteProperty* properties = nullptr;
};

using AsepriteUUID = u8[16];

enum AsepritePropertyType : u16
{
    AsepriteProperty_Bool           = 0x0001,
    AsepriteProperty_Int8           = 0x0002,
    AsepriteProperty_UInt8          = 0x0003,
    AsepriteProperty_Int16          = 0x0004,
    AsepriteProperty_UInt16         = 0x0005,
    AsepriteProperty_Int32          = 0x0006,
    AsepriteProperty_UInt32         = 0x0007,
    AsepriteProperty_Int64          = 0x0008,
    AsepriteProperty_UInt64         = 0x0009,
    AsepriteProperty_Fixed          = 0x000A,
    AsepriteProperty_Float          = 0x000B,
    AsepriteProperty_Double         = 0x000C,
    AsepriteProperty_String         = 0x000D,
    AsepriteProperty_Point          = 0x000E,
    AsepriteProperty_Size           = 0x000F,
    AsepriteProperty_Rect           = 0x0010,
    AsepriteProperty_Array          = 0x0011,
    AsepriteProperty_PropertiesMap  = 0x0012,
    AsepriteProperty_UUID           = 0x0013,
};

static constexpr i64 aseprite_type_size_in_bytes[] = {
    -1,                             // Invalid
    sizeof(u8),                     // Bool
    sizeof(i8),                     // Int8
    sizeof(u8),                     // UInt8
    sizeof(i16),                    // Int16
    sizeof(u16),                    // UInt16
    sizeof(i32),                    // Int32
    sizeof(u32),                    // UInt32
    sizeof(i64),                    // Int64
    sizeof(u64),                    // UInt64
    sizeof(AsepriteFixed),          // Fixed
    sizeof(f32),                    // Float
    sizeof(f64),                    // Double
    sizeof(AsepriteString),         // String
    sizeof(AsepritePoint),          // Point
    sizeof(AsepriteSize),           // Size
    sizeof(AsepriteRect),           // Rect
    sizeof(AsepriteArray),          // Array
    sizeof(AsepritePropertiesMap),  // PropertiesMap
    sizeof(AsepriteUUID),           // UUID
};

struct AsepriteProperty
{
    AsepriteString name;
    AsepritePropertyType type = (AsepritePropertyType)0;

    union
    {
        bool bool_value = 0; // == 0 means FALSE, != 0 means TRUE
        i8 int8_value;
        u8 uint8_value;
        i16 int16_value;
        u16 uint16_value;
        i32 int32_value;
        u32 uint32_value;
        i64 int64_value;
        u64 uint64_value;
        AsepriteFixed fixed_value;
        f32 float_value;
        f64 double_value;
        AsepriteString string_value;
        AsepritePoint point_value;
        AsepriteSize size_value;
        AsepriteRect rect_value;
        AsepriteArray array_value;
        AsepritePropertiesMap properties_map_value;
        AsepriteUUID uuid_value;
    };
};

struct AsepriteUserDataChunkData
{
    AsepriteUserDataFlags flags = 0;

    // If flags have bit 1 (has text)
    AsepriteString text;

    // If flags have bit 2 (has color)
    AsepriteColor color;

    // If flags have bit 4 (has properties)
    u32 size_in_bytes = 0; // size in bytes of all properties maps stored in this chunk; the size includes the this field and the number of property maps (so it will be a value greater or equal to 8 bytes)
    u32 properties_map_count = 0;
    AsepritePropertiesMap* properties_maps = nullptr;
};

struct AsepriteSliceChunkData
{
    struct SliceKey
    {
        struct Header
        {
            u32 frame_number = 0;   // this slice is valid from this frame to the end of the animation
            i32 x_origin_in_sprite = 0;
            i32 y_origin_in_sprite = 0;
            u32 width = 0;          // can be 0 if this slice hidden in the animation from the given frame
            u32 height = 0;
        } header;

        struct Center
        {
            i32 x_position = 0; // relative to slice bounds
            i32 y_position = 0;
            u32 width = 0;
            u32 height = 0;
        } center; // if flags have bit 1 (it's a 9-patches slice)

        struct Pivot
        {
            i32 x_position = 0; // relative to the slice origin
            i32 y_position = 0; // relative to the slice origin
        } pivot; // if flags have bit 2 (has pivot information)
    };

    struct Header
    {
        u32 slice_key_count = 0;
        u32 flags = 0; // 1 = it's a 9-patches slice, 2 = has pivot information
        u32 _unused = 0;
    } header;

    AsepriteString name;
    SliceKey* slice_keys = nullptr;
};

struct AsepriteTilesetChunkData
{
    struct Header
    {
        u32 id = 0;
        u32 flags = 0;              // 1 - include link to external file
                                    // 2 - include tiles inside this file
                                    // 4 - tilemaps using this tileset use tile ID=0 as empty tile (this is the new format); in rare cases this bit is off, and the empty tile will be equal to 0xffffffff (used in internal versions of Aseprite)
                                    // 8 - aseprite will try to match modified tiles with their X flipped version automatically in Auto mode when using this tileset
                                    // 16 - same for Y flips
                                    // 32 - same for D(iagonal) flips
        u32 count = 0;
        u16 tile_width = 0;
        u16 tile_height = 0;
        i16 base_index = 0;         // number to show in the screen from the tile with index 1 and so on (by default this is field is 1, so the data that is displayed is equivalent to the data in memory); but it can be 0 to display zero-based indexing (this field isn't used for the representation of the data in the file, it's just for UI purposes)
        u8 _unused[14] = { 0 };     // reserved
    } header;

    AsepriteString name;

    // If flag 1 is set (include link to external file)
    u32 id_of_external_file = 0;    // this ID is one entry of the the External Files Chunk
    u32 id_in_external_file = 0;

    // If flag 2 is set (include tiles inside this file)
    u32 data_length = 0;            // length of the compressed Tileset image
    u8* compressed_image = nullptr; // (tile widht) x (tile height x number of tiles)
};

enum AsepriteChunkType : u16
{
    AsepriteChunk_OldPalette        = 0x0004,
    AsepriteChunk_OldPaletteSmall   = 0x0011,
    AsepriteChunk_Layer             = 0x2004,
    AsepriteChunk_Cel               = 0x2005,
    AsepriteChunk_CelExtra          = 0x2006,
    AsepriteChunk_ColorProfile      = 0x2007,
    AsepriteChunk_ExternalFiles     = 0x2008,
    AsepriteChunk_Mask              = 0x2016, // DEPRECATED
    AsepriteChunk_Path              = 0x2017, // never used
    AsepriteChunk_Tags              = 0x2018,
    AsepriteChunk_Palette           = 0x2019,
    AsepriteChunk_UserData          = 0x2020,
    AsepriteChunk_Slice             = 0x2022,
    AsepriteChunk_Tileset           = 0x2023,
};

struct AsepriteChunk
{
    struct Header
    {
        u32 size = 0; // includes the DWORD of the size itself, and the WORD of the chunk type, so a chunk size must be equal or greater than 6 bytes at least
        AsepriteChunkType type = (AsepriteChunkType)0;
    } header;

    union
    {
        AsepriteOldPaletteChunkData* old_palette_data = nullptr;
        AsepriteLayerChunkData* layer_data;
        AsepriteCelChunkData* cel_data;
        AsepriteCelExtraChunkData* cel_extra_data;
        AsepriteColorProfileChunkData* color_profile_data;
        AsepriteExternalFilesChunkData* external_files_data;
        AsepriteMaskChunkData* mask_data; // DEPRECATED
        AsepritePathChunkData* path_data; // never used
        AsepriteTagsChunkData* tags_data;
        AsepritePaletteChunkData* palette_data;
        AsepriteUserDataChunkData* user_data;
        AsepriteSliceChunkData* slice_data;
        AsepriteTilesetChunkData* tileset_data;
    };
};

struct AsepriteFrame
{
    struct Header
    {
        u32 size = 0;               // bytes in this frame
        u16 magic_number = 0;       // always 0xF1FA
        u16 chunk_count_old = 0;    // old field which specifies the number of "chunks" in this frame; if this value is 0xFFFF, we might have more chunks to read in this frame (so we have to use the new field)
        u16 duration = 0;           // in milliseconds
        u8 _unused[2] = { 0 };      // for future (set to zero)
        u32 chunk_count_new = 0;    // new field which specifies the number of "chunks" in this frame (if this is 0, use the old field)
    } header;

    AsepriteChunk* chunks = nullptr;
};

struct AsepriteFile
{
    struct Header
    {
        u32 file_size = 0;
        u16 magic_number = 0;       // 0xA5E0
        u16 frame_count = 0;
        u16 width = 0;
        u16 height = 0;
        u16 bits_per_pixel = 0;     // color depth: 32 bpp = RGBA, 16 bpp = Grayscale, 8 bpp = Indexed
        u32 flags = 0;              // 1 = layer opacity has valid value
        u16 speed = 0;              // milliseconds between frame, like in FLC files [DEPRECATED: You should use the frame duration field from each frame header]
        u32 _unused1 = 0;           // set be 0
        u32 _unused2 = 0;           // set be 0
        u8 pallete_entry_index = 0; // which represent transparent color in all non-background layers (only for Indexed sprites)
        u8 _unused3[3] = { 0 };     // ignore these bytes
        u16 number_of_colors = 0;   // 0 means 256 for old sprites
        u8 pixel_width = 0;         // pixel ratio is "pixel width/pixel height"; if this or pixel height field is zero, pixel ratio is 1:1
        u8 pixel_height = 0;
        i16 x_position_of_grid = 0;
        i16 y_position_of_grid = 0;
        u16 grid_width = 0;         // zero if there is no grid, grid size is 16x16 on Aseprite by default
        u16 grid_height = 0;        // zero if there is no grid
        u8 _unused4[84] = { 0 };    // for future (set to zero)
    } header;

    AsepriteFrame* frames = nullptr;
};

#pragma pack(pop)

struct BinaryFile
{
    String buffer;
    u64 offset = 0;
};

static void read_from_file(BinaryFile* file, void* out_value, i64 size)
{
    void* data = file->buffer.data + file->offset;
    file->offset += size;

    memcpy(out_value, data, size);
}

template<typename T>
static void read_from_file(BinaryFile* file, T* out_value)
{
    i64 size = sizeof(T);
    read_from_file(file, out_value, size);
}

template<typename T>
static void read_array_from_file(BinaryFile* file, MemoryArena* arena, T** out_array, i64 count)
{
    i64 size = sizeof(T) * count;
    void* data = file->buffer.data + file->offset;
    file->offset += size;

    *out_array = arena_push_uninitialized<T>(arena, count);
    memcpy(*out_array, data, size);
}

static void read_string_from_file(BinaryFile* file, AsepriteString* string, MemoryArena* arena)
{
    read_from_file(file, &string->count);

    if (string->count == 0)
    {
        string->data = nullptr;
    }
    else
    {
        read_array_from_file(file, arena, &string->data, string->count);
    }
}

static void* zlib_alloc(void* opaque, u32 items, u32 size)
{
    MemoryArena* arena = (MemoryArena*)opaque;
    return arena_push(arena, items * size);
}

static void zlib_free(void* opaque, void* address)
{
}

static u8* uncompress_data_from_file(BinaryFile* file, MemoryArena* arena)
{
    z_stream strm;
    strm.zalloc = zlib_alloc;
    strm.zfree = zlib_free;

    strm.next_in = file->buffer.data + file->offset;
    strm.avail_in = file->buffer.length - file->offset;

    i64 space_left = arena->max - arena->pos;
    u8* result = arena_push_uninitialized<u8>(arena, space_left);

    strm.next_out = result;
    strm.avail_out = space_left;

    ScratchArena temp = scratch_begin(arena);
    {
        strm.opaque = temp.arena;

        i32 result = inflateInit(&strm);
        APORIA_ASSERT(result == Z_OK);

        result = inflate(&strm, Z_FINISH);
        APORIA_ASSERT(result == Z_STREAM_END);

        inflateEnd(&strm);
    }
    scratch_end(temp);

    arena_pop(arena, space_left - strm.total_out);
    file->offset += strm.total_in;

    return result;
}

static void read_properties_from_file(BinaryFile* file, MemoryArena* arena, AsepriteProperty* properties, u32 property_count)
{
    for (u32 i = 0; i < property_count; ++i)
    {
        AsepriteProperty* property = &properties[i];
        read_from_file(file, &property->name);
        read_from_file(file, &property->type);

        switch (property->type)
        {
            case AsepriteProperty_Bool:     read_from_file(file, &property->bool_value);    break;
            case AsepriteProperty_Int8:     read_from_file(file, &property->int8_value);    break;
            case AsepriteProperty_UInt8:    read_from_file(file, &property->uint8_value);   break;
            case AsepriteProperty_Int16:    read_from_file(file, &property->int16_value);   break;
            case AsepriteProperty_UInt16:   read_from_file(file, &property->uint16_value);  break;
            case AsepriteProperty_Int32:    read_from_file(file, &property->int32_value);   break;
            case AsepriteProperty_UInt32:   read_from_file(file, &property->uint32_value);  break;
            case AsepriteProperty_Int64:    read_from_file(file, &property->int64_value);   break;
            case AsepriteProperty_UInt64:   read_from_file(file, &property->uint64_value);  break;
            case AsepriteProperty_Fixed:    read_from_file(file, &property->fixed_value);   break;
            case AsepriteProperty_Float:    read_from_file(file, &property->float_value);   break;
            case AsepriteProperty_Double:   read_from_file(file, &property->double_value);  break;
            case AsepriteProperty_String:   read_from_file(file, &property->string_value);  break;
            case AsepriteProperty_Point:    read_from_file(file, &property->point_value);   break;
            case AsepriteProperty_Size:     read_from_file(file, &property->size_value);    break;
            case AsepriteProperty_Rect:     read_from_file(file, &property->rect_value);    break;

            case AsepriteProperty_Array:
            {
                AsepriteArray* array = &property->array_value;

                read_from_file(file, &array->count);
                read_from_file(file, &array->type);

                if (array->type == 0)
                {
                    array->types = arena_push_uninitialized<u16>(arena, array->count);

                    i64 max_needed_size = arena->max - arena->pos;
                    array->elements = arena_push_uninitialized<u8>(arena, max_needed_size);

                    i64 actual_needed_size = 0;

                    for (u32 n = 0; n < array->count; ++n)
                    {
                        u16* type = &array->types[n];
                        read_from_file(file, type);

                        APORIA_ASSERT(*type != AsepriteProperty_Array && *type != AsepriteProperty_PropertiesMap);
                        i64 type_size_in_bytes = aseprite_type_size_in_bytes[*type];

                        read_from_file(file, array->elements + actual_needed_size, type_size_in_bytes);

                        actual_needed_size += type_size_in_bytes;
                    }

                    arena_pop(arena, max_needed_size - actual_needed_size);
                }
                else
                {
                    i64 type_size_in_bytes = aseprite_type_size_in_bytes[array->type];
                    array->elements = arena_push_uninitialized<u8>(arena, array->count * type_size_in_bytes);

                    read_from_file(file, array->elements, array->count * type_size_in_bytes);
                }
            }
            break;

            case AsepriteProperty_PropertiesMap:
            {
                AsepritePropertiesMap* properties_map = &property->properties_map_value;
                read_from_file(file, &properties_map->property_count);

                properties_map->properties = arena_push<AsepriteProperty>(arena, properties_map->property_count);

                read_properties_from_file(file, arena, properties_map->properties, properties_map->property_count);
            }
            break;

            case AsepriteProperty_UUID: read_from_file(file, property->uuid_value); break;
        }
    }
}

// For details, see https://github.com/aseprite/aseprite/blob/main/docs/ase-file-specs.md
static AsepriteFile load_aseprite_file(MemoryArena* arena, String filepath)
{
    ScratchArena temp = scratch_begin(arena);

    BinaryFile file;
    file.buffer = read_entire_file(temp.arena, filepath);

    AsepriteFile aseprite;
    read_from_file(&file, &aseprite.header);
    APORIA_ASSERT(aseprite.header.magic_number == 0xA5E0);

    aseprite.frames = arena_push<AsepriteFrame>(arena, aseprite.header.frame_count);

    for (u16 i = 0; i < aseprite.header.frame_count; ++i)
    {
        AsepriteFrame* frame = &aseprite.frames[i];
        read_from_file(&file, &frame->header);
        APORIA_ASSERT(frame->header.magic_number == 0xF1FA);

        u16 chunk_count = frame->header.chunk_count_new;
        if (chunk_count == 0)
            chunk_count = frame->header.chunk_count_old;

        frame->chunks = arena_push<AsepriteChunk>(arena, chunk_count);

        for (u16 j = 0; j < chunk_count; ++j)
        {
            AsepriteChunk* chunk = &frame->chunks[j];
            read_from_file(&file, &chunk->header);

            switch (chunk->header.type)
            {
                case AsepriteChunk_OldPalette:
                case AsepriteChunk_OldPaletteSmall:
                {
                    auto* data = chunk->old_palette_data = arena_push_uninitialized<AsepriteOldPaletteChunkData>(arena);
                    read_from_file(&file, &data->packet_count);

                    data->packets = arena_push_uninitialized<AsepriteOldPaletteChunkData::Packet>(arena, data->packet_count);

                    for (u16 k = 0; k < data->packet_count; ++k)
                    {
                        AsepriteOldPaletteChunkData::Packet* packet = &data->packets[k];

                        read_from_file(&file, &packet->palette_entries_to_skip_from_last_packet_count);
                        read_from_file(&file, &packet->color_count);

                        u16 color_count = packet->color_count;
                        if (color_count == 0)
                            color_count = 256;

                        packet->colors = arena_push_uninitialized<AsepriteOldPaletteChunkData::Color>(arena, color_count);
                        read_array_from_file(&file, arena, &packet->colors, color_count);
                    }
                }
                break;

                case AsepriteChunk_Layer:
                {
                    auto* data = chunk->layer_data = arena_push<AsepriteLayerChunkData>(arena);
                    read_from_file(&file, &data->header);

                    read_string_from_file(&file, &data->layer_name, arena);

                    if (data->header.type == AsepriteLayer_Tilemap)
                        read_from_file(&file, &data->tileset_index);
                }
                break;

                case AsepriteChunk_Cel:
                {
                    auto* data = chunk->cel_data = arena_push<AsepriteCelChunkData>(arena);
                    read_from_file(&file, &data->header);

                    switch (data->header.cel_type)
                    {
                        case AsepriteCel_RawImageData:
                        {
                            auto* raw_image_data = &data->raw_image_data;

                            read_from_file(&file, &raw_image_data->width_in_pixels);
                            read_from_file(&file, &raw_image_data->height_in_pixels);

                            i64 pixel_count = raw_image_data->width_in_pixels * raw_image_data->height_in_pixels;
                            i64 pixels_size = pixel_count * (aseprite.header.bits_per_pixel / 8);

                            raw_image_data->raw_pixels = arena_push_uninitialized<u8>(arena, pixels_size);
                            read_array_from_file(&file, arena, &raw_image_data->raw_pixels, pixels_size);
                        }
                        break;

                        case AsepriteCel_LinkedCel:
                        {
                            read_from_file(&file, &data->linked_cel.frame_position_to_link_with);
                        }
                        break;

                        case AsepriteCel_CompressedImage:
                        {
                            auto* compressed_image = &data->compressed_image;

                            read_from_file(&file, &compressed_image->width_in_pixels);
                            read_from_file(&file, &compressed_image->height_in_pixels);

                            compressed_image->raw_cel = uncompress_data_from_file(&file, arena);
                        }
                        break;

                        case AsepriteCel_CompressedTilemap:
                        {
                            auto* compressed_tilemap = &data->compressed_tilemap;

                            read_from_file(&file, &compressed_tilemap->header);

                            compressed_tilemap->tiles = uncompress_data_from_file(&file, arena);
                        }
                        break;
                    }
                }
                break;

                case AsepriteChunk_CelExtra:
                {
                    chunk->cel_extra_data = arena_push_uninitialized<AsepriteCelExtraChunkData>(arena);
                    read_from_file(&file, chunk->cel_extra_data);
                }
                break;

                case AsepriteChunk_ColorProfile:
                {
                    auto* data = chunk->color_profile_data = arena_push<AsepriteColorProfileChunkData>(arena);
                    read_from_file(&file, &data->header);

                    if (data->header.type == 2) // ICC
                    {
                        read_from_file(&file, &data->icc_profile_data_length);
                        read_array_from_file(&file, arena, &data->icc_profile_data, data->icc_profile_data_length);
                    }
                }
                break;

                case AsepriteChunk_ExternalFiles:
                {
                    auto* data = chunk->external_files_data = arena_push_uninitialized<AsepriteExternalFilesChunkData>(arena);
                    read_from_file(&file, &data->header);

                    data->entries = arena_push_uninitialized<AsepriteExternalFilesChunkData::Entry>(arena, data->header.entry_count);
                    read_array_from_file(&file, arena, &data->entries, data->header.entry_count);

                    for (u32 k = 0; k < data->header.entry_count; ++k)
                    {
                        AsepriteExternalFilesChunkData::Entry* entry = &data->entries[k];
                        read_from_file(&file, &entry->header);

                        read_string_from_file(&file, &entry->external_file_name_or_extension_id, arena);
                    }
                }
                break;

                case AsepriteChunk_Mask: // DEPRECATED
                {
                    auto* data = chunk->mask_data = arena_push_uninitialized<AsepriteMaskChunkData>(arena);
                    read_from_file(&file, &data->header);

                    read_string_from_file(&file, &data->name, arena);

                    i64 bit_map_data_size = data->header.width * ((data->header.height + 7) / 8);
                    read_array_from_file(&file, arena, &data->bit_map_data, bit_map_data_size);
                }
                break;

                case AsepriteChunk_Path: // never used
                {
                }
                break;

                case AsepriteChunk_Tags:
                {
                    auto* data = chunk->tags_data = arena_push_uninitialized<AsepriteTagsChunkData>(arena);
                    read_from_file(&file, &data->header);

                    data->tags = arena_push_uninitialized<AsepriteTagsChunkData::Tag>(arena, data->header.tag_count);

                    for (u32 k = 0; k < data->header.tag_count; ++k)
                    {
                        AsepriteTagsChunkData::Tag* tag = &data->tags[k];
                        read_from_file(&file, &tag->header);

                        read_string_from_file(&file, &tag->name, arena);
                    }
                }
                break;

                case AsepriteChunk_Palette:
                {
                    auto* data = chunk->palette_data = arena_push<AsepritePaletteChunkData>(arena);
                    read_from_file(&file, &data->header);

                    data->palette_entries = arena_push<AsepritePaletteChunkData::PaletteEntry>(arena, data->header.new_palette_size);

                    for (u32 k = 0; k < data->header.new_palette_size; ++k)
                    {
                        AsepritePaletteChunkData::PaletteEntry* palette_entry = &data->palette_entries[k];
                        read_from_file(&file, &palette_entry->header);

                        if (palette_entry->header.flags & 0x0001)
                        {
                            read_string_from_file(&file, &palette_entry->color_name, arena);
                        }
                    }
                }
                break;

                case AsepriteChunk_UserData:
                {
                    auto* data = chunk->user_data = arena_push<AsepriteUserDataChunkData>(arena);
                    read_from_file(&file, &data->flags);

                    if (data->flags & AsepriteUserDataFlag_HasText)
                    {
                        read_string_from_file(&file, &data->text, arena);
                    }

                    if (data->flags & AsepriteUserDataFlag_HasColor)
                    {
                        read_from_file(&file, &data->color);
                    }

                    if (data->flags & AsepriteUserDataFlag_HasProperties)
                    {
                        read_from_file(&file, &data->size_in_bytes);
                        read_from_file(&file, &data->properties_map_count);

                        data->properties_maps = arena_push_uninitialized<AsepritePropertiesMap>(arena, data->properties_map_count);

                        for (u32 k = 0; k < data->properties_map_count; ++k)
                        {
                            AsepritePropertiesMap* properties_map = &data->properties_maps[k];
                            read_from_file(&file, &properties_map->key);
                            read_from_file(&file, &properties_map->property_count);

                            properties_map->properties = arena_push<AsepriteProperty>(arena, properties_map->property_count);

                            read_properties_from_file(&file, arena, properties_map->properties, properties_map->property_count);
                        }
                    }
                }
                break;

                case AsepriteChunk_Slice:
                {
                    auto* data = chunk->slice_data = arena_push_uninitialized<AsepriteSliceChunkData>(arena);
                    read_from_file(&file, &data->header);

                    read_string_from_file(&file, &data->name, arena);

                    data->slice_keys = arena_push<AsepriteSliceChunkData::SliceKey>(arena, data->header.slice_key_count);

                    for (u32 k = 0; k < data->header.slice_key_count; ++k)
                    {
                        AsepriteSliceChunkData::SliceKey* slice = &data->slice_keys[k];
                        read_from_file(&file, &slice->header);

                        if (data->header.flags & 0x00000001) // it's a 9-pathces slice
                        {
                            read_from_file(&file, &slice->center);
                        }

                        if (data->header.flags & 0x00000002) // has pivot information
                        {
                            read_from_file(&file, &slice->pivot);
                        }
                    }
                }
                break;

                case AsepriteChunk_Tileset:
                {
                    auto* data = chunk->tileset_data = arena_push<AsepriteTilesetChunkData>(arena);
                    read_from_file(&file, &data->header);

                    read_string_from_file(&file, &data->name, arena);

                    if (data->header.flags & 0x00000001)
                    {
                        read_from_file(&file, &data->id_of_external_file);
                        read_from_file(&file, &data->id_in_external_file);
                    }

                    if (data->header.flags & 0x00000002)
                    {
                        read_from_file(&file, &data->data_length);

                        uncompress_data_from_file(&file, arena);
                    }
                }
                break;
            }
        }
    }

    scratch_end(temp);

    return aseprite;
}

// @TODO(dubgron): Add generating spritesheets from .aseprite files.
