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

    namespace ConfigParser
    {
        static u8 comment_token = ';';

        using TokenType = u8;
        enum TokenType_ : TokenType
        {
            TokenType_None      = 0x00,
            TokenType_Comment   = 0x01,
            TokenType_Category  = 0x02,
            TokenType_Field     = 0x04,
            TokenType_Literal   = 0x08,

            TokenType_Any       = 0xff,
        };

        struct Token
        {
            TokenType type = TokenType_None;
            String text;
        };

        struct TokenNode
        {
            TokenNode* next = nullptr;
            Token token;
        };

        struct TokenList
        {
            TokenNode* first = nullptr;
            TokenNode* last = nullptr;
            u64 node_count = 0;

            void push_node(MemoryArena* arena, Token token);
        };

        void TokenList::push_node(MemoryArena* arena, Token token)
        {
            TokenNode* node = arena->push_zero<TokenNode>();
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

        static u8 expected_tokens_table(TokenType token)
        {
            switch (token)
            {
            case TokenType_None:        return TokenType_Comment | TokenType_Category;
            case TokenType_Comment:     return TokenType_Any;
            case TokenType_Category:    return TokenType_Comment | TokenType_Field;
            case TokenType_Field:       return TokenType_Comment | TokenType_Literal;
            case TokenType_Literal:     return TokenType_Any;

            default: APORIA_UNREACHABLE(); return TokenType_None;
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
            if (flag & TokenType_Comment)
            {
                valid_token_types.push_node(arena, create_string("Comment"));
            }
            if (flag & TokenType_Category)
            {
                valid_token_types.push_node(arena, create_string("Category"));
            }
            if (flag & TokenType_Field)
            {
                valid_token_types.push_node(arena, create_string("Field"));
            }
            if (flag & TokenType_Literal)
            {
                valid_token_types.push_node(arena, create_string("Literal"));
            }

            return valid_token_types.join(arena, create_string(" or "));
        }
    }

    bool load_config(std::string_view filepath)
    {
        ScratchArena temp = create_scratch_arena(&persistent_arena);

        std::string config_contents_str = read_file(filepath);
        String config_contents = String{ (u8*)config_contents_str.data(), config_contents_str.size() };

        // Tokenizing
        String buffer = config_contents;
        u8 expected_tokens = expected_tokens_table(ConfigParser::TokenType_None);

        ConfigParser::TokenList token_list;

        ConfigParser::consume_whitespace(&buffer);
        while (buffer.length > 0)
        {
            const String token_string = ConfigParser::consume_token(&buffer);

            const u8 first_char = token_string.data[0];
            const u8 last_char = token_string.data[token_string.length - 1];

            ConfigParser::Token token;
            if ((expected_tokens & ConfigParser::TokenType_Comment) && first_char == ConfigParser::comment_token)
            {
                ConfigParser::consume_whitespace(&buffer);
                token.text = ConfigParser::consume_until(&buffer, '\n');
                token.type = ConfigParser::TokenType_Comment;
            }
            else if ((expected_tokens & ConfigParser::TokenType_Category) && first_char == '[' && last_char == ']')
            {
                token.text = token_string.substr(1, token_string.length - 2);
                token.type = ConfigParser::TokenType_Category;
            }
            else if ((expected_tokens & ConfigParser::TokenType_Literal) && first_char == '"' && last_char == '"')
            {
                token.text = token_string.substr(1, token_string.length - 2);
                token.type = ConfigParser::TokenType_Literal;
            }
            else if ((expected_tokens & ConfigParser::TokenType_Literal) && (first_char == '-' || isdigit(first_char)))
            {
                token.text = token_string;
                token.type = ConfigParser::TokenType_Literal;
            }
            else if ((expected_tokens & ConfigParser::TokenType_Literal) && (token_string == "true" || token_string == "false"))
            {
                token.text = token_string;
                token.type = ConfigParser::TokenType_Literal;
            }
            else if (expected_tokens & ConfigParser::TokenType_Field)
            {
                token.text = token_string;
                token.type = ConfigParser::TokenType_Field;
            }

            if (token.type == ConfigParser::TokenType_None)
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
                APORIA_LOG(Error, "Error at line: {}, column: {}. Expected token type: {}, but got: >>> {} <<<!", line, column, ConfigParser::print_token_type_flag(temp.arena, expected_tokens), token_string);
                return false;
            }

            switch (token.type)
            {
            case ConfigParser::TokenType_Comment:     APORIA_LOG(Debug, "TYPE: Comment,  VALUE: {}", token.text); break;
            case ConfigParser::TokenType_Category:    APORIA_LOG(Debug, "TYPE: Category, VALUE: {}", token.text); break;
            case ConfigParser::TokenType_Field:       APORIA_LOG(Debug, "TYPE: Field,    VALUE: {}", token.text); break;
            case ConfigParser::TokenType_Literal:     APORIA_LOG(Debug, "TYPE: Literal,  VALUE: {}", token.text); break;
            }

            token_list.push_node(temp.arena, token);
            expected_tokens = ConfigParser::expected_tokens_table(token.type);

            ConfigParser::consume_whitespace(&buffer);
        }

        APORIA_LOG(Debug, "Tokenized {} tokens!", token_list.node_count);

        // Parsing
        String category;
        String field;
        StringList literals;

        for (ConfigParser::TokenNode* token_node = token_list.first; token_node; token_node = token_node->next)
        {
            ConfigParser::Token token = token_node->token;
            switch (token.type)
            {
            case ConfigParser::TokenType_Comment: break;

            case ConfigParser::TokenType_Category:
            {
                category = token.text;
            }
            break;

            case ConfigParser::TokenType_Field:
            {
                field = token.text;
            }
            break;

            case ConfigParser::TokenType_Literal:
            {
                literals.push_node(temp.arena, token.text);

                const bool next_token_is_part_of_current_field = token_node->next && token_node->next->token.type & expected_tokens_table(ConfigParser::TokenType_Field);
                if (!next_token_is_part_of_current_field)
                {
                    APORIA_LOG(Debug, "{} {} = {}", category, field, literals.join(temp.arena, create_string(", ")));

                    // Applying properties
                    if (category == "window")
                    {
                        if (field == "title")
                        {
                            window_config.title = literals.get_node_at_index(0)->string;
                        }
                        else if (field == "size")
                        {
                            window_config.width = literals.get_node_at_index(0)->string.to_int();
                            window_config.height = literals.get_node_at_index(1)->string.to_int();
                        }
                        else if (field == "vsync")
                        {
                            window_config.vsync = literals.get_node_at_index(0)->string.to_bool();
                        }
                        else if (field == "position")
                        {
                            window_config.position->x = literals.get_node_at_index(0)->string.to_int();
                            window_config.position->y = literals.get_node_at_index(1)->string.to_int();
                        }
                    }
                    else if (category == "camera")
                    {
                        if (field == "fov")
                        {
                            camera_config.fov = literals.get_node_at_index(0)->string.to_float();
                        }
                        else if (field == "aspect_ratio")
                        {
                            camera_config.aspect_ratio = literals.get_node_at_index(0)->string.to_float();
                        }
                        else if (field == "background_color")
                        {
                            camera_config.background_color.r = literals.get_node_at_index(0)->string.to_int();
                            camera_config.background_color.g = literals.get_node_at_index(1)->string.to_int();
                            camera_config.background_color.b = literals.get_node_at_index(2)->string.to_int();
                            camera_config.background_color.a = literals.get_node_at_index(3)->string.to_int();
                        }
                        else if (field == "movement_speed")
                        {
                            camera_config.movement_speed = literals.get_node_at_index(0)->string.to_float();
                        }
                        else if (field == "rotation_speed")
                        {
                            camera_config.rotation_speed = literals.get_node_at_index(0)->string.to_float();
                        }
                        else if (field == "zoom_speed")
                        {
                            camera_config.zoom_speed = literals.get_node_at_index(0)->string.to_float();
                        }
                        else if (field == "movement_key_up")
                        {
                            camera_config.movement_key_up = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "movement_key_down")
                        {
                            camera_config.movement_key_down = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "movement_key_left")
                        {
                            camera_config.movement_key_left = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "movement_key_down")
                        {
                            camera_config.movement_key_down = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "movement_key_right")
                        {
                            camera_config.movement_key_right = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "rotation_key_left")
                        {
                            camera_config.rotation_key_left = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "rotation_key_right")
                        {
                            camera_config.rotation_key_right = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "zoom_key_in")
                        {
                            camera_config.zoom_key_in = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "zoom_key_out")
                        {
                            camera_config.zoom_key_out = string_to_key(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "zoom_max")
                        {
                            camera_config.zoom_max = literals.get_node_at_index(0)->string.to_float();
                        }
                        else if (field == "zoom_min")
                        {
                            camera_config.zoom_min = literals.get_node_at_index(0)->string.to_float();
                        }
                    }
                    else if (category == "shader")
                    {
                        if (field == "default.blend")
                        {
                            shader_config.default_properties.blend[0] = string_to_shader_blend(literals.get_node_at_index(0)->string);
                            shader_config.default_properties.blend[1] = string_to_shader_blend(literals.get_node_at_index(1)->string);
                        }
                        else if (field == "default.blend_op")
                        {
                            shader_config.default_properties.blend_op = string_to_shader_blend_op(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "default.depth_test")
                        {
                            shader_config.default_properties.depth_test = string_to_shader_depth_test(literals.get_node_at_index(0)->string);
                        }
                        else if (field == "default.depth_write")
                        {
                            shader_config.default_properties.depth_write = string_to_shader_depth_write(literals.get_node_at_index(0)->string);
                        }
                    }

                    literals.clear();
                }
            }
            break;
            }
        }

        rollback_scratch_arena(temp);

        return true;
    }
}
