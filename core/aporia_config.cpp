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
    EditorConfig editor_config;
    CameraConfig camera_config;

    static u8 comment_token     = ';';
    static u8 array_begin_token = '(';
    static u8 array_end_token   = ')';

    enum Config_TokenType : u8
    {
        Config_TokenType_None       = 0x00,
        Config_TokenType_Comment    = 0x01,
        Config_TokenType_Category   = 0x02,
        Config_TokenType_Field      = 0x04,
        Config_TokenType_Literal    = 0x08,
        Config_TokenType_ArrayBegin = 0x10,
        Config_TokenType_ArrayEnd   = 0x20,

        Config_TokenType_Any        = 0xff,
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
            case Config_TokenType_None:         return Config_TokenType_Comment | Config_TokenType_Category;
            case Config_TokenType_Comment:      return Config_TokenType_Any;
            case Config_TokenType_Category:     return Config_TokenType_Comment | Config_TokenType_Field;
            case Config_TokenType_Field:        return Config_TokenType_Comment | Config_TokenType_Literal | Config_TokenType_ArrayBegin;
            case Config_TokenType_Literal:      return Config_TokenType_Any;
            // @NOTE(dubgron): The following line forbids empty arrays.
            case Config_TokenType_ArrayBegin:   return Config_TokenType_Comment | Config_TokenType_Literal | Config_TokenType_ArrayBegin;
            case Config_TokenType_ArrayEnd:     return Config_TokenType_Any;

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
        const u64 eol = buffer->find_eol();
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
            return chr == comment_token || chr == array_begin_token || chr == array_end_token;
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
            const u8 chr = buffer->data[token_length];

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
        if (flag & Config_TokenType_ArrayBegin)
        {
            valid_token_types.push_node(arena, "ArrayBegin");
        }
        if (flag & Config_TokenType_ArrayEnd)
        {
            valid_token_types.push_node(arena, "ArrayEnd");
        }

        return valid_token_types.join(arena, " or ");
    }

    static StringList literals_list_to_string_list(MemoryArena* arena, const Config_LiteralsList& list)
    {
        StringList result;
        for (Config_LiteralsNode* node = list.first; node; node = node->next)
        {
            if (node->array != nullptr)
            {
                StringNode* before_array = result.last;

                StringList array_literals = literals_list_to_string_list(arena, *node->array);
                result.append(array_literals);

                if (before_array)
                {
                    APORIA_ASSERT(before_array->next);
                    before_array->next->string = before_array->next->string.append_front(arena, "(");
                }
                else
                {
                    APORIA_ASSERT(result.first);
                    result.first->string = result.first->string.append_front(arena, "(");
                }

                APORIA_ASSERT(result.last);
                result.last->string = result.last->string.append(arena, ")");
                result.total_length += 2;
            }
            else
            {
                result.push_node(arena, node->literal);
            }

            if (node->next && result.last)
            {
                result.last->string = result.last->string.append(arena, ",");
                result.total_length += 1;
            }
        }
        return result;
    }

    Config_LiteralsNode* Config_LiteralsList::push_node(MemoryArena* arena)
    {
        Config_LiteralsNode* node = arena->push_zero<Config_LiteralsNode>();

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

        return node;
    }

    void Config_LiteralsList::clear()
    {
        next = prev = nullptr;
        first = last = nullptr;
        node_count = 0;
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

#define PRINT_TOKEN(arena, token) do { \
        ScratchArena temp = create_scratch_arena(arena); \
        APORIA_LOG(Verbose, "Type: {:12} Token: '{}'", token_type_flag_to_string(temp.arena, token.type), token.text); \
        rollback_scratch_arena(temp); \
    } while(0)

#define PRINT_PROPERTY(arena, property) do { \
        ScratchArena temp = create_scratch_arena(arena); \
        APORIA_LOG(Verbose, "{}.{} = {}", property.category, property.field, literals_list_to_string_list(temp.arena, property.literals).join(temp.arena, " ")); \
        rollback_scratch_arena(temp); \
    } while (0)

    Config_PropertyList parse_config_from_file(MemoryArena* arena, String filepath)
    {
        String config_contents = read_file(arena, filepath);

        // Tokenization / Lexing
        String buffer = config_contents;
        consume_whitespace(&buffer);

        u8 expected_tokens = expected_tokens_table(Config_TokenType_None);

        Config_TokenList token_list;
        while (buffer.length > 0)
        {
            const String token_string = consume_token(&buffer);

            const u8 first_char = token_string.data[0];
            const u8 last_char = token_string.data[token_string.length - 1];

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
            else if ((expected_tokens & Config_TokenType_ArrayBegin) && token_string == "(")
            {
                token.type = Config_TokenType_ArrayBegin;
            }
            else if ((expected_tokens & Config_TokenType_ArrayEnd) && token_string == ")")
            {
                token.type = Config_TokenType_ArrayEnd;
            }
            else if (expected_tokens & Config_TokenType_Field)
            {
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

                ScratchArena temp = create_scratch_arena(arena);
                APORIA_LOG(Error, "Syntax error at line: {}, column: {}. Expected token type: {}, but got: >>> {} <<<!",
                    line, column, token_type_flag_to_string(temp.arena, expected_tokens), token_string);
                rollback_scratch_arena(temp);

                return Config_PropertyList{};
            }

            PRINT_TOKEN(arena, token);

            token_list.push_node(arena, token);
            expected_tokens = expected_tokens_table(token.type);

            consume_whitespace(&buffer);
        }

        APORIA_LOG(Debug, "Tokenization completed! Processed {} tokens!", token_list.node_count);

        // Parsing
        Config_PropertyList property_list;

        Config_Property property;
        Config_LiteralsList* active_literals = &property.literals;

        for (Config_TokenNode* token_node = token_list.first; token_node; token_node = token_node->next)
        {
            Config_Token token = token_node->token;

            switch (token.type)
            {
                case Config_TokenType_Comment: break;

                case Config_TokenType_Category:
                {
                    property.category = token.text;
                }
                break;

                case Config_TokenType_Field:
                {
                    property.field = token.text;
                }
                break;

                case Config_TokenType_Literal:
                {
                    Config_LiteralsNode* new_literal = active_literals->push_node(arena);

                    new_literal->literal = token.text;
                }
                break;

                case Config_TokenType_ArrayBegin:
                {
                    Config_LiteralsNode* new_literal = active_literals->push_node(arena);

                    new_literal->array = arena->push_zero<Config_LiteralsList>();

                    new_literal->array->prev = active_literals;
                    active_literals = new_literal->array;
                }
                break;

                case Config_TokenType_ArrayEnd:
                {
                    APORIA_ASSERT(active_literals->prev != nullptr);

                    // Flatten the array, if it only has one element.
                    if (active_literals->node_count == 1)
                    {
                        APORIA_ASSERT(active_literals->prev->last->array == active_literals);
                        *active_literals->prev->last = *active_literals->first;
                    }

                    active_literals = active_literals->prev;
                    active_literals->next = nullptr;
                }
                break;
            }

            // If in process of adding literals, let's check if we should stop and submit.
            if (property.literals.node_count > 0)
            {
                constexpr u8 tokens_allowed_in_literals = Config_TokenType_Comment | Config_TokenType_Literal | Config_TokenType_ArrayBegin | Config_TokenType_ArrayEnd;

                // If there is no more literals, submit the property and clear the list of literals.
                if (!token_node->next || !(token_node->next->token.type & tokens_allowed_in_literals))
                {
                    APORIA_ASSERT(active_literals == &property.literals);

                    // Remove an outer array, if needed.
                    if (property.literals.node_count == 1 && property.literals.first->array != nullptr)
                    {
                        property.literals = *property.literals.first->array;
                    }

                    PRINT_PROPERTY(arena, property);

                    property_list.push_node(arena, property);

                    property.literals.clear();
                }

            }
        }

        APORIA_LOG(Debug, "Parsing completed! Processed {} properties!", property_list.node_count);

        return property_list;
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
        { "camera", "aspect_ratio",           Config_ValueType_Float32,           1, &camera_config.aspect_ratio },
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

        { "editor", "display_editor_grid",    Config_ValueType_Boolean,           1, &editor_config.display_editor_grid },
    };
    static constexpr u64 defined_properties_count = sizeof(defined_properties) / sizeof(Config_PropertyDefinition);

#define PROPERTY_HELPER(T, string_to_type) do { \
        T* property_data = (T*)property_definition.data; \
        Config_LiteralsNode* literal_node = literals.first; \
        for (u64 value_idx = 0; value_idx < property_definition.value_count; ++value_idx) \
        { \
            property_data[value_idx] = string_to_type(literal_node->literal); \
            literal_node = literal_node->next; \
        } \
    } while(0)

    bool load_engine_config(String filepath)
    {
        ScratchArena temp = create_scratch_arena(&persistent_arena);
        Config_PropertyList parsed_config = parse_config_from_file(temp.arena, filepath);

        if (parsed_config.node_count == 0)
        {
            rollback_scratch_arena(temp);
            return false;
        }

        for (Config_PropertyNode* property_node = parsed_config.first; property_node; property_node = property_node->next)
        {
            String category = property_node->property.category;
            String field = property_node->property.field;
            Config_LiteralsList literals = property_node->property.literals;

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
                        APORIA_LOG(Error, "Field {}.{} has a literal count mismatch (expected: {}, received: {})!", category, field, property_definition.value_count, literals.node_count);
                        continue;
                    }

                    constexpr auto copy_string = [](String string) { return push_string(&config_arena, string); };

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

        rollback_scratch_arena(temp);

        return true;
    }

#undef PROPERTY_HELPER
}
