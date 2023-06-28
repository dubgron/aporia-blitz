#include "aporia_config.hpp"

#include "aporia_debug.hpp"
#include "aporia_entity.hpp"
#include "aporia_game.hpp"
#include "aporia_inputs.hpp"
#include "aporia_shaders.hpp"
#include "aporia_strings.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    WindowConfig window_config;
    ShaderConfig shader_config;
    CameraConfig camera_config;

    static u8 comment_token = ';';

    using Config_TokenType = u8;
    enum Config_TokenType_ : Config_TokenType
    {
        Config_TokenType_None      = 0x00,
        Config_TokenType_Comment   = 0x01,
        Config_TokenType_Category  = 0x02,
        Config_TokenType_Field     = 0x04,
        Config_TokenType_Literal   = 0x08,

        Config_TokenType_Any       = 0xff,
    };

    struct Config_Token
    {
        Config_TokenType type = Config_TokenType_None;
        String text;
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
        Config_TokenNode* node = arena->push_zero<Config_TokenNode>();
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
            case Config_TokenType_None:        return Config_TokenType_Comment | Config_TokenType_Category;
            case Config_TokenType_Comment:     return Config_TokenType_Any;
            case Config_TokenType_Category:    return Config_TokenType_Comment | Config_TokenType_Field;
            case Config_TokenType_Field:       return Config_TokenType_Comment | Config_TokenType_Literal;
            case Config_TokenType_Literal:     return Config_TokenType_Any;

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

    static String consume_until(String* buffer, u8 end)
    {
        String result = *buffer;
        while (buffer->length > 0 && *buffer->data != end)
        {
            buffer->data += 1;
            buffer->length -= 1;
        }
        result.length -= buffer->length;
        return result;
    }

    static String consume_token(String* buffer)
    {
        if (*buffer->data == comment_token)
        {
            String token{ buffer->data, 1 };
            buffer->data += 1;
            buffer->length -= 1;
            return token;
        }

        String token = *buffer;
        bool ignore_special_chars = false;
        while (buffer->length > 0 && (ignore_special_chars ||
            !(isspace(*buffer->data) || *buffer->data == comment_token)))
        {
            if (*buffer->data == '"')
            {
                ignore_special_chars = !ignore_special_chars;
            }

            buffer->data += 1;
            buffer->length -= 1;
        }
        token.length -= buffer->length;
        return token;
    }

    static String print_token_type_flag(MemoryArena* arena, u8 flag)
    {
        StringList valid_token_types;
        if (flag & Config_TokenType_Comment)
        {
            valid_token_types.push_node(arena, create_string("Comment"));
        }
        if (flag & Config_TokenType_Category)
        {
            valid_token_types.push_node(arena, create_string("Category"));
        }
        if (flag & Config_TokenType_Field)
        {
            valid_token_types.push_node(arena, create_string("Field"));
        }
        if (flag & Config_TokenType_Literal)
        {
            valid_token_types.push_node(arena, create_string("Literal"));
        }

        return valid_token_types.join(arena, create_string(" or "));
    }

    void Config_PropertyList::push_node(MemoryArena* arena, Config_Property property)
    {
        Config_PropertyNode* node = arena->push_zero<Config_PropertyNode>();
        node->property = property;

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

    Config_PropertyList parse_config_file(std::string_view filepath)
    {
        ScratchArena temp = create_scratch_arena(&persistent_arena);

        std::string config_contents_str = read_file(filepath);
        String config_contents = String{ (u8*)config_contents_str.data(), config_contents_str.size() };

        // Tokenizing
        String buffer = config_contents;
        u8 expected_tokens = expected_tokens_table(Config_TokenType_None);

        Config_TokenList token_list;

        consume_whitespace(&buffer);
        while (buffer.length > 0)
        {
            const String token_string = consume_token(&buffer);

            const u8 first_char = token_string.data[0];
            const u8 last_char = token_string.data[token_string.length - 1];

            Config_Token token;
            if ((expected_tokens & Config_TokenType_Comment) && first_char == comment_token)
            {
                consume_whitespace(&buffer);
                token.text = consume_until(&buffer, '\n');
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
                token.text = token_string;
                token.type = Config_TokenType_Literal;
            }
            else if ((expected_tokens & Config_TokenType_Literal) && (token_string == "true" || token_string == "false"))
            {
                token.text = token_string;
                token.type = Config_TokenType_Literal;
            }
            else if (expected_tokens & Config_TokenType_Field)
            {
                token.text = token_string;
                token.type = Config_TokenType_Field;
            }

            if (token.type == Config_TokenType_None)
            {
                u64 line = 1;
                u64 column = 1;
                const u64 error_at_location = config_contents.length - buffer.length - token_string.length;
                for (u64 idx = 0; idx < error_at_location; ++idx)
                {
                    column += 1;

                    if (config_contents.data[idx] == '\n')
                    {
                        line += 1;
                        column = 1;
                    }
                }
                APORIA_LOG(Error, "Error at line: {}, column: {}. Expected token type: {}, but got: >>> {} <<<!", line, column, print_token_type_flag(temp.arena, expected_tokens), token_string);
                return Config_PropertyList{};
            }

            switch (token.type)
            {
                case Config_TokenType_Comment:     APORIA_LOG(Debug, "TYPE: Comment,  VALUE: {}", token.text); break;
                case Config_TokenType_Category:    APORIA_LOG(Debug, "TYPE: Category, VALUE: {}", token.text); break;
                case Config_TokenType_Field:       APORIA_LOG(Debug, "TYPE: Field,    VALUE: {}", token.text); break;
                case Config_TokenType_Literal:     APORIA_LOG(Debug, "TYPE: Literal,  VALUE: {}", token.text); break;
            }

            token_list.push_node(temp.arena, token);
            expected_tokens = expected_tokens_table(token.type);

            consume_whitespace(&buffer);
        }

        APORIA_LOG(Debug, "Tokenized {} tokens!", token_list.node_count);

        // Parsing
        Config_PropertyList result;
        Config_Property property;

        for (Config_TokenNode* token_node = token_list.first; token_node; token_node = token_node->next)
        {
            Config_Token token = token_node->token;
            String copied_token_text = push_string(&config_arena, token.text);

            switch (token.type)
            {
                case Config_TokenType_Comment: break;

                case Config_TokenType_Category:
                {
                    property.category = copied_token_text;
                }
                break;

                case Config_TokenType_Field:
                {
                    property.field = copied_token_text;
                }
                break;

                case Config_TokenType_Literal:
                {
                    property.literals.push_node(temp.arena, copied_token_text);

                    const bool next_token_is_part_of_current_field = token_node->next && token_node->next->token.type & expected_tokens_table(Config_TokenType_Field);
                    if (!next_token_is_part_of_current_field)
                    {
                        result.push_node(&config_arena, property);

                        APORIA_LOG(Debug, "{} -> {} = {}", property.category, property.field, property.literals.join(temp.arena, create_string(", ")));
                        property.literals.clear();
                    }
                }
                break;
            }
        }

        rollback_scratch_arena(temp);

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

#define str(x) create_string(x)

    static Config_PropertyDefinition defined_properties[] =
    {
        { str("window"), str("title"),                  Config_ValueType_String,            1, &window_config.title },
        { str("window"), str("size"),                   Config_ValueType_Int32,             2, &window_config.width },
        { str("window"), str("vsync"),                  Config_ValueType_Boolean,           1, &window_config.vsync },
        { str("window"), str("position"),               Config_ValueType_Int32,             2, &window_config.position },

        { str("camera"), str("fov"),                    Config_ValueType_Float32,           1, &camera_config.fov },
        { str("camera"), str("aspect_ratio"),           Config_ValueType_Float32,           1, &camera_config.aspect_ratio },
        { str("camera"), str("background_color"),       Config_ValueType_Uint8,             4, &camera_config.background_color },
        { str("camera"), str("movement_speed"),         Config_ValueType_Float32,           1, &camera_config.movement_speed },
        { str("camera"), str("rotation_speed"),         Config_ValueType_Float32,           1, &camera_config.rotation_speed },
        { str("camera"), str("zoom_speed"),             Config_ValueType_Float32,           1, &camera_config.zoom_speed },
        { str("camera"), str("movement_key_up"),        Config_ValueType_Key,               1, &camera_config.movement_key_up },
        { str("camera"), str("movement_key_down"),      Config_ValueType_Key,               1, &camera_config.movement_key_down },
        { str("camera"), str("movement_key_left"),      Config_ValueType_Key,               1, &camera_config.movement_key_left },
        { str("camera"), str("movement_key_right"),     Config_ValueType_Key,               1, &camera_config.movement_key_right },
        { str("camera"), str("rotation_key_left"),      Config_ValueType_Key,               1, &camera_config.rotation_key_left },
        { str("camera"), str("rotation_key_right"),     Config_ValueType_Key,               1, &camera_config.rotation_key_right },
        { str("camera"), str("zoom_key_in"),            Config_ValueType_Key,               1, &camera_config.zoom_key_in },
        { str("camera"), str("zoom_key_out"),           Config_ValueType_Key,               1, &camera_config.zoom_key_out },
        { str("camera"), str("zoom_max"),               Config_ValueType_Float32,           1, &camera_config.zoom_max },
        { str("camera"), str("zoom_min"),               Config_ValueType_Float32,           1, &camera_config.zoom_min },

        { str("shader"), str("default.blend"),          Config_ValueType_ShaderBlend,       2, &shader_config.default_properties.blend },
        { str("shader"), str("default.blend_op"),       Config_ValueType_ShaderBlendOp,     1, &shader_config.default_properties.blend_op },
        { str("shader"), str("default.depth_test"),     Config_ValueType_ShaderDepthTest,   1, &shader_config.default_properties.depth_test },
        { str("shader"), str("default.depth_write"),    Config_ValueType_ShaderDepthWrite,  1, &shader_config.default_properties.depth_write },
    };
    static constexpr u64 defined_properties_count = sizeof(defined_properties) / sizeof(Config_PropertyDefinition);

#undef str

#define PROPERTY_HELPER(T, string_to_type) do { \
        T* property_data = (T*)property_definition.data; \
        StringNode* value_node = literals.first; \
        for (u64 value_idx = 0; value_idx < property_definition.value_count; ++value_idx) \
        { \
            property_data[value_idx] = string_to_type(value_node->string); \
            value_node = value_node->next; \
        } \
    } while(0)

    bool load_engine_config(std::string_view filepath)
    {
        Config_PropertyList parsed_config = parse_config_file(filepath);

        if (parsed_config.first == nullptr)
        {
            return false;
        }
        
        for (Config_PropertyNode* property_node = parsed_config.first; property_node; property_node = property_node->next)
        {
            String category = property_node->property.category;
            String field = property_node->property.field;
            StringList literals = property_node->property.literals;

            // Applying properties
            // @TODO(dubgron): The big-O notation of this solution is not great. Use a hash map to improve it.
            for (u64 idx = 0; idx < defined_properties_count; ++idx)
            {
                const Config_PropertyDefinition& property_definition = defined_properties[idx];
                if (property_definition.category == category && property_definition.field == field)
                {
                    if (literals.node_count != property_definition.value_count)
                    {
                        // @TODO(dubgron): Better logging.
                        APORIA_LOG(Error, "Field has a literal count mismatch!");
                        continue;
                    }

                    switch (property_definition.value_type)
                    {
                        case Config_ValueType_String:           { PROPERTY_HELPER(String, );                                        } break;
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

        return true;
    }

#undef PROPERTY_HELPER
}
