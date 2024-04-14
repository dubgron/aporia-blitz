#include "aporia_config.hpp"

#include "aporia_camera.hpp"
#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_rendering.hpp"
#include "aporia_utils.hpp"
#include "aporia_window.hpp"

WindowConfig window_config;
RenderingConfig rendering_config;
ShaderConfig shader_config;
CameraConfig camera_config;

#if defined(APORIA_EDITOR)
EditorConfig editor_config;
#endif

static u8 comment_token                 = ';';
static u8 inner_property_begin_token    = '(';
static u8 inner_property_end_token      = ')';

enum Config_TokenType : u8
{
    Config_TokenType_None               = 0x00,

    Config_TokenType_Comment            = 0x01,
    Config_TokenType_Category           = 0x02,
    Config_TokenType_Field              = 0x04,
    Config_TokenType_Literal            = 0x08,
    Config_TokenType_InnerPropertyBegin = 0x10,
    Config_TokenType_InnerPropertyEnd   = 0x20,

    Config_TokenType_Any                = 0xff,
};

struct Config_Token
{
    String text;
    Config_TokenType type = Config_TokenType_None;
};

struct Config_TokenNode
{
    Config_TokenNode* next = nullptr;
    Config_Token token;
};

struct Config_TokenList
{
    Config_TokenNode* first = nullptr;
    Config_TokenNode* last = nullptr;
    u64 node_count = 0;

    void push_node(MemoryArena* arena, Config_Token token);
};

void Config_TokenList::push_node(MemoryArena* arena, Config_Token token)
{
    Config_TokenNode* node = arena_push<Config_TokenNode>(arena);
    node->token = token;

    if (node_count > 0)
    {
        last->next = node;
    }
    else
    {
        first = node;
    }

    last = node;
    node_count += 1;
}

static u8 expected_tokens_table(Config_TokenType token)
{
    switch (token)
    {
        case Config_TokenType_None:                 return Config_TokenType_Comment | Config_TokenType_Category;
        case Config_TokenType_Comment:              return Config_TokenType_Any;
        case Config_TokenType_Category:             return Config_TokenType_Comment | Config_TokenType_Field;
        case Config_TokenType_Field:                return Config_TokenType_Comment | Config_TokenType_Literal | Config_TokenType_InnerPropertyBegin;
        case Config_TokenType_Literal:              return Config_TokenType_Comment | Config_TokenType_Category | Config_TokenType_Field | Config_TokenType_Literal | Config_TokenType_InnerPropertyEnd;
        case Config_TokenType_InnerPropertyBegin:   return Config_TokenType_Comment | Config_TokenType_Field;
        case Config_TokenType_InnerPropertyEnd:     return Config_TokenType_Comment | Config_TokenType_Category | Config_TokenType_Field | Config_TokenType_InnerPropertyEnd;

        default: APORIA_UNREACHABLE(); return Config_TokenType_None;
    }
};

static void consume_whitespace(String* buffer)
{
    while (buffer->length > 0 && isspace(*buffer->data))
    {
        buffer->data += 1;
        buffer->length -= 1;
    }
}

static String consume_until_eol(String* buffer)
{
    u64 eol = buffer->find_eol();
    String result = buffer->substr(0, eol);

    buffer->data += eol;
    buffer->length -= eol;

    return result;
}

// @NOTE(dubgron): Remember to call consume_whitespace before calling this!
static String consume_token(String* buffer)
{
    constexpr auto is_special_token = [](u8 chr)
    {
        return chr == comment_token || chr == inner_property_begin_token || chr == inner_property_end_token;
    };

    if (is_special_token(buffer->data[0]))
    {
        String token{ buffer->data, 1 };
        buffer->data += 1;
        buffer->length -= 1;
        return token;
    }

    u64 token_length = 0;
    bool ignore_special_chars = false;

    while (token_length < buffer->length)
    {
        u8 chr = buffer->data[token_length];

        if (!ignore_special_chars && (isspace(chr) || is_special_token(chr)))
        {
            break;
        }

        if (chr == '"')
        {
            ignore_special_chars = !ignore_special_chars;
        }

        token_length += 1;
    }

    String token{ buffer->data, token_length };
    buffer->data += token_length;
    buffer->length -= token_length;
    return token;
}

static String token_type_flag_to_string(MemoryArena* arena, u8 flag)
{
    StringList valid_token_types;
    if (flag & Config_TokenType_Comment)
    {
        valid_token_types.push_node(arena, "Comment");
    }
    if (flag & Config_TokenType_Category)
    {
        valid_token_types.push_node(arena, "Category");
    }
    if (flag & Config_TokenType_Field)
    {
        valid_token_types.push_node(arena, "Field");
    }
    if (flag & Config_TokenType_Literal)
    {
        valid_token_types.push_node(arena, "Literal");
    }
    if (flag & Config_TokenType_InnerPropertyBegin)
    {
        valid_token_types.push_node(arena, "InnerPropertyBegin");
    }
    if (flag & Config_TokenType_InnerPropertyEnd)
    {
        valid_token_types.push_node(arena, "InnerPropertyEnd");
    }

    return valid_token_types.join(arena, " or ");
}

Config_Property* parse_config_from_file(MemoryArena* arena, String filepath)
{
    String config_contents = read_entire_text_file(arena, filepath);

    // Tokenization / Lexing
    String buffer = config_contents;
    consume_whitespace(&buffer);

    // @TODO(dubgron): Enforcing the order of tokens using expected_tokens
    // should be probably done in the parsing stage as it will make giving
    // better syntax error messages more convinient.
    u8 expected_tokens = expected_tokens_table(Config_TokenType_None);

    Config_TokenList token_list;
    while (buffer.length > 0)
    {
        String token_string = consume_token(&buffer);

        u8 first_char = token_string.data[0];
        u8 last_char = token_string.data[token_string.length - 1];

        Config_Token token{ token_string };
        if ((expected_tokens & Config_TokenType_Comment) && first_char == comment_token)
        {
            consume_whitespace(&buffer);
            token.text = consume_until_eol(&buffer);
            token.type = Config_TokenType_Comment;
        }
        else if ((expected_tokens & Config_TokenType_Category) && first_char == '[' && last_char == ']')
        {
            token.text = token_string.substr(1, token_string.length - 2);
            token.type = Config_TokenType_Category;
        }
        else if ((expected_tokens & Config_TokenType_Literal) && first_char == '"' && last_char == '"')
        {
            token.text = token_string.substr(1, token_string.length - 2);
            token.type = Config_TokenType_Literal;
        }
        else if ((expected_tokens & Config_TokenType_Literal) && (first_char == '-' || isdigit(first_char)))
        {
            token.type = Config_TokenType_Literal;
        }
        else if ((expected_tokens & Config_TokenType_Literal) && (token_string == "true" || token_string == "false"))
        {
            token.type = Config_TokenType_Literal;
        }
        else if ((expected_tokens & Config_TokenType_InnerPropertyBegin) && token_string == "(")
        {
            token.type = Config_TokenType_InnerPropertyBegin;
        }
        else if ((expected_tokens & Config_TokenType_InnerPropertyEnd) && token_string == ")")
        {
            token.type = Config_TokenType_InnerPropertyEnd;
        }
        else if (expected_tokens & Config_TokenType_Field)
        {
            token.type = Config_TokenType_Field;
        }

        if (token.type == Config_TokenType_None)
        {
            u64 line = 1;
            u64 column = 1;
            u64 error_at_location = config_contents.length - buffer.length - token_string.length;
            for (u64 idx = 0; idx < error_at_location; ++idx)
            {
                column += 1;

                if (config_contents.data[idx] == '\n')
                {
                    line += 1;
                    column = 1;
                }
            }

            ScratchArena temp = scratch_begin(arena);
            APORIA_LOG(Error, "Syntax error at line: %, column: %. Expected token type: %, but got: >>> % <<<!",
                line, column, token_type_flag_to_string(temp.arena, expected_tokens), token_string);
            scratch_end(temp);

            return nullptr;
        }

        ScratchArena temp = scratch_begin(arena);
        APORIA_LOG(Verbose, "Type: % Token: '%'", token_type_flag_to_string(temp.arena, token.type), token.text);
        scratch_end(temp);

        token_list.push_node(arena, token);
        expected_tokens = expected_tokens_table(token.type);

        consume_whitespace(&buffer);
    }

    APORIA_LOG(Debug, "Tokenization completed! Processed % tokens!", token_list.node_count);

    // Parsing
    Config_Property* result = arena_push<Config_Property>(arena);

    Config_Property* property = result;
    u64 total_property_count = 0;

    Config_TokenNode* token_node = token_list.first;
    while (token_node != nullptr)
    {
        const Config_Token& token = token_node->token;

        if (property->inner != nullptr || property->literals.node_count > 0)
        {
            APORIA_ASSERT(token.type != Config_TokenType_InnerPropertyBegin);

            // If property->inner is not null (meaning we've just encountered
            // an InnerPropertyEnd token), the next token can't be a Literal.
            APORIA_ASSERT(property->inner == nullptr || token.type != Config_TokenType_Literal);

            // If the next token is either a Category or a Field, a new property is needed.
            if (token.type & (Config_TokenType_Category | Config_TokenType_Field))
            {
                property->next = arena_push<Config_Property>(arena);
                property->next->category = property->category;
                property->next->outer = property->outer;

                property->next->prev = property;
                property = property->next;

                total_property_count += 1;
            }
        }

        switch (token.type)
        {
            case Config_TokenType_Comment: break;

            case Config_TokenType_Category:
            {
                property->category = token.text;
            }
            break;

            case Config_TokenType_Field:
            {
                property->field = token.text;
            }
            break;

            case Config_TokenType_Literal:
            {
                property->literals.push_node(arena, token.text);
            }
            break;

            case Config_TokenType_InnerPropertyBegin:
            {
                property->inner = arena_push<Config_Property>(arena);
                property->inner->outer = property;

                property = property->inner;
            }
            break;

            case Config_TokenType_InnerPropertyEnd:
            {
                APORIA_ASSERT(property->outer != nullptr);

                property = property->outer;
            }
            break;
        }

        token_node = token_node->next;
    }

    APORIA_ASSERT(property != nullptr && property->outer == nullptr);
    APORIA_LOG(Debug, "Parsing completed! Processed % properties!", total_property_count);

    return result;
}

enum Config_ValueType
{
    Config_ValueType_String,
    Config_ValueType_Key,
    Config_ValueType_ShaderBlend,
    Config_ValueType_ShaderBlendOp,
    Config_ValueType_ShaderDepthTest,
    Config_ValueType_ShaderDepthWrite,
    Config_ValueType_Float32,
    Config_ValueType_Float64,
    Config_ValueType_Int8,
    Config_ValueType_Int16,
    Config_ValueType_Int32,
    Config_ValueType_Int64,
    Config_ValueType_Uint8,
    Config_ValueType_Uint16,
    Config_ValueType_Uint32,
    Config_ValueType_Uint64,
    Config_ValueType_Boolean,
};

struct Config_PropertyDefinition
{
    String category;
    String field;

    Config_ValueType value_type;
    u64 value_count = 0;

    void* data = nullptr;
};

static Config_PropertyDefinition defined_properties[] =
{
    { "window", "title",                  Config_ValueType_String,            1, &window_config.title },
    { "window", "size",                   Config_ValueType_Int32,             2, &window_config.width },
    { "window", "vsync",                  Config_ValueType_Boolean,           1, &window_config.vsync },
    { "window", "position",               Config_ValueType_Int32,             2, &window_config.position },

    { "camera", "fov",                    Config_ValueType_Float32,           1, &camera_config.fov },
    { "camera", "background_color",       Config_ValueType_Uint8,             4, &camera_config.background_color },
    { "camera", "movement_speed",         Config_ValueType_Float32,           1, &camera_config.movement_speed },
    { "camera", "rotation_speed",         Config_ValueType_Float32,           1, &camera_config.rotation_speed },
    { "camera", "zoom_speed",             Config_ValueType_Float32,           1, &camera_config.zoom_speed },
    { "camera", "movement_key_up",        Config_ValueType_Key,               1, &camera_config.movement_key_up },
    { "camera", "movement_key_down",      Config_ValueType_Key,               1, &camera_config.movement_key_down },
    { "camera", "movement_key_left",      Config_ValueType_Key,               1, &camera_config.movement_key_left },
    { "camera", "movement_key_right",     Config_ValueType_Key,               1, &camera_config.movement_key_right },
    { "camera", "rotation_key_left",      Config_ValueType_Key,               1, &camera_config.rotation_key_left },
    { "camera", "rotation_key_right",     Config_ValueType_Key,               1, &camera_config.rotation_key_right },
    { "camera", "zoom_key_in",            Config_ValueType_Key,               1, &camera_config.zoom_key_in },
    { "camera", "zoom_key_out",           Config_ValueType_Key,               1, &camera_config.zoom_key_out },
    { "camera", "zoom_max",               Config_ValueType_Float32,           1, &camera_config.zoom_max },
    { "camera", "zoom_min",               Config_ValueType_Float32,           1, &camera_config.zoom_min },

    { "shader", "default.blend",          Config_ValueType_ShaderBlend,       2, &shader_config.default_properties.blend },
    { "shader", "default.blend_op",       Config_ValueType_ShaderBlendOp,     1, &shader_config.default_properties.blend_op },
    { "shader", "default.depth_test",     Config_ValueType_ShaderDepthTest,   1, &shader_config.default_properties.depth_test },
    { "shader", "default.depth_write",    Config_ValueType_ShaderDepthWrite,  1, &shader_config.default_properties.depth_write },

    { "rendering", "custom_resolution",   Config_ValueType_Int32,             2, &rendering_config.custom_resolution_width },

#if defined(APORIA_EDITOR)
    { "editor", "display_editor_grid",    Config_ValueType_Boolean,           1, &editor_config.display_editor_grid },
#endif
};
static constexpr u64 defined_properties_count = ARRAY_COUNT(defined_properties);

#define PROPERTY_HELPER(T, string_to_type) do { \
    T* property_data = (T*)property_definition.data; \
    StringNode* literal_node = property->literals.first; \
    for (u64 value_idx = 0; value_idx < property_definition.value_count; ++value_idx) \
    { \
        property_data[value_idx] = string_to_type(literal_node->string); \
        literal_node = literal_node->next; \
    } \
} while(0)

static bool load_engine_config_from_file(String filepath)
{
    ScratchArena temp = scratch_begin();
    Config_Property* parsed_config = parse_config_from_file(temp.arena, filepath);

    if (!parsed_config)
    {
        scratch_end(temp);
        return false;
    }

    for (Config_Property* property = parsed_config; property; property = property->next)
    {
        // Applying properties
        // @TODO(dubgron): The big-O notation of this solution is not great. Use a hash map to improve it.
        for (u64 idx = 0; idx < defined_properties_count; ++idx)
        {
            const Config_PropertyDefinition& property_definition = defined_properties[idx];
            if (property_definition.category == property->category && property_definition.field == property->field)
            {
                if (property->literals.node_count != property_definition.value_count)
                {
                    // @TODO(dubgron): Better logging.
                    APORIA_LOG(Error, "Property %.% has a literal count mismatch (expected: %, received: %)!",
                        property->category, property->field, property_definition.value_count, property->literals.node_count);
                    continue;
                }

                constexpr auto copy_string = [](String string) { return push_string(&memory.config, string); };

                switch (property_definition.value_type)
                {
                    case Config_ValueType_String:           { PROPERTY_HELPER(String, copy_string);                             } break;
                    case Config_ValueType_Key:              { PROPERTY_HELPER(Key, string_to_key);                              } break;
                    case Config_ValueType_ShaderBlend:      { PROPERTY_HELPER(ShaderBlend, string_to_shader_blend);             } break;
                    case Config_ValueType_ShaderBlendOp:    { PROPERTY_HELPER(ShaderBlendOp, string_to_shader_blend_op);        } break;
                    case Config_ValueType_ShaderDepthTest:  { PROPERTY_HELPER(ShaderDepthTest, string_to_shader_depth_test);    } break;
                    case Config_ValueType_ShaderDepthWrite: { PROPERTY_HELPER(ShaderDepthWrite, string_to_shader_depth_write);  } break;
                    case Config_ValueType_Float32:          { PROPERTY_HELPER(f32, string_to_float);                            } break;
                    case Config_ValueType_Float64:          { PROPERTY_HELPER(f64, string_to_float);                            } break;
                    case Config_ValueType_Int8:             { PROPERTY_HELPER(i8, string_to_int);                               } break;
                    case Config_ValueType_Int16:            { PROPERTY_HELPER(i16, string_to_int);                              } break;
                    case Config_ValueType_Int32:            { PROPERTY_HELPER(i32, string_to_int);                              } break;
                    case Config_ValueType_Int64:            { PROPERTY_HELPER(i64, string_to_int);                              } break;
                    case Config_ValueType_Uint8:            { PROPERTY_HELPER(u8, string_to_int);                               } break;
                    case Config_ValueType_Uint16:           { PROPERTY_HELPER(u16, string_to_int);                              } break;
                    case Config_ValueType_Uint32:           { PROPERTY_HELPER(u32, string_to_int);                              } break;
                    case Config_ValueType_Uint64:           { PROPERTY_HELPER(u64, string_to_int);                              } break;
                    case Config_ValueType_Boolean:          { PROPERTY_HELPER(bool, string_to_bool);                            } break;
                }
            }
        }
    }

    scratch_end(temp);

    return true;
}

#undef PROPERTY_HELPER

bool reload_config_asset(Asset* config_asset)
{
    APORIA_ASSERT(config_asset->type == AssetType::Config);

    arena_clear(&memory.config);

    // Reset configs to perform a clean reload.
    window_config = {};
    rendering_config = {};
    shader_config = {};
    camera_config = {};

#if defined(APORIA_EDITOR)
    editor_config = {};
#endif

    bool success = load_engine_config_from_file(config_asset->source_file);
    config_asset->status = success ? AssetStatus::Loaded : AssetStatus::Unloaded;

    window_apply_config();
    active_camera->apply_config();

    adjust_framebuffers_to_render_surface();

    return success;
}

bool load_engine_config(String filepath)
{
    bool success = load_engine_config_from_file(filepath);

    Asset* config_asset = register_asset(filepath, AssetType::Config);
    config_asset->status = success ? AssetStatus::Loaded : AssetStatus::Unloaded;

    return success;
}
