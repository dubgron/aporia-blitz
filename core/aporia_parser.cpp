#include "aporia_parser.hpp"

#include "aporia_debug.hpp"
#include "aporia_utils.hpp"

static bool is_newline(u8 c)
{
    return c == '\n' || c == '\r';
}

static bool is_space(u8 c)
{
    return c == ' ' || c == '\t' || is_newline(c);
}

static bool is_alpha(u8 c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool is_number(u8 c)
{
    return c >= '0' && c <= '9';
}

static bool is_hexadecimal(u8 c)
{
    return is_number(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static bool is_alphanumeric(u8 c)
{
    return is_alpha(c) || is_number(c);
}

static bool can_start_identifier(u8 c)
{
    return is_alpha(c) || c == '_';
}

static bool can_continue_identifier(u8 c)
{
    return can_start_identifier(c) || is_number(c);
}

static bool can_start_number(u8 c)
{
    return is_number(c) || c == '-' || c == '+';
}

enum TokenKind : i16
{
    Token_Invalid       = -1,

    Token_Comment       = ';',
    Token_CategoryBegin = '[',
    Token_CategoryEnd   = ']',
    Token_StructBegin   = '{',
    Token_StructEnd     = '}',

    Token_Identifier    = 256,
    Token_Number,
    Token_String,

    Token_Keyword_True,
    Token_Keyword_False,

    Token_EndOfFile,
};

struct Token
{
    TokenKind type = Token_Invalid;
    ValueFlags value_flags = ValueFlag_None;

    i32 pos = -1;

    union
    {
        i64 int_value = 0;
        u64 uint_value;
        f64 float64_value;
        f32 float32_value;
        String string_value;
        bool bool_value;
    };

    // @HACK(dubgron): Until Clang-17 and GCC-13, those compilers didn't properly
    // compile anonymous unions of structs without default constructors.
    Token() {};
};

struct Lexer
{
    String buffer;
    i64 cursor = 0;

    Token last_token;
    bool last_token_available = false;

    bool error_occured = false;

    String source_filepath;
};

struct TokenInfo
{
    i32 line_number = 0;
    i16 column_begin = 0;
    i16 column_end = 0;

    String context;
};

#if defined(APORIA_DEBUGTOOLS)

static TokenInfo find_token_in_buffer(Lexer* lexer, Token* token)
{
    TokenInfo result;

    i64 line_begin, line_end = INDEX_INVALID;
    do
    {
        line_begin = line_end + 1;
        line_end = lexer->buffer.find_eol(line_begin);
        result.line_number += 1;
    } while (line_end < token->pos);

    result.column_begin = token->pos - line_begin + 1;
    result.column_end = lexer->cursor - line_begin;

    constexpr i64 max_context_lines = 2;
    i64 context_lines_count = 0;

    while (line_begin > 0 && context_lines_count < max_context_lines)
    {
        i64 prev_begin = lexer->buffer.rfind('\n', line_begin - 2) + 1;
        String prev_line = lexer->buffer.substr(prev_begin, line_begin - prev_begin);

        bool is_line_empty = true;
        for (i64 idx = 0; idx < prev_line.length; ++idx)
        {
            if (!is_space(prev_line.data[idx]))
            {
                is_line_empty = false;
                break;
            }
        }

        if (is_line_empty)
            break;

        line_begin = prev_begin;
        context_lines_count += 1;
    }

    result.context = lexer->buffer.substr(line_begin, line_end - line_begin);

    return result;
}

template<typename... Ts>
static void report_parsing_error(Lexer* lexer, Token* token, String format, Ts&&... args)
{
    if (lexer->error_occured)
        return;

    TokenInfo token_info = find_token_in_buffer(lexer, token);

    log(lexer->source_filepath, token_info.line_number, token_info.column_begin, Error, format, args...);

    log(Error, "");
    log(Error, token_info.context);

    ScratchArena temp = scratch_begin();
    {
        String underline = push_string(temp.arena, token_info.column_end);
        memset(underline.data, ' ', token_info.column_begin - 1);
        memset(underline.data + token_info.column_begin - 1, '^', token_info.column_end - token_info.column_begin + 1);
        log(Error, underline);
        log(Error, "");
    }
    scratch_end(temp);

    lexer->error_occured = true;
}

static void report_parsing_error(Lexer* lexer, String message)
{
    report_parsing_error(lexer, &lexer->last_token, message);
}

static void report_detail(Lexer* lexer, String message)
{
    if (!lexer->error_occured)
        return;

    TokenInfo token_info = find_token_in_buffer(lexer, &lexer->last_token);
    log(lexer->source_filepath, token_info.line_number, token_info.column_begin, Error, message);
}

#else

#define find_token_in_buffer(...)
#define report_parsing_error(...)
#define report_detail(...)

#endif

static u8 peek_next_character(Lexer* lexer)
{
    return lexer->cursor < lexer->buffer.length ? lexer->buffer.data[lexer->cursor] : 0;
}

static void consume_character(Lexer* lexer)
{
    lexer->cursor += 1;
}

static void consume_whitespace(Lexer* lexer)
{
    u8 c = peek_next_character(lexer);
    while (is_space(c))
    {
        consume_character(lexer);
        c = peek_next_character(lexer);
    }
}

static void consume_until_new_line(Lexer* lexer)
{
    u8 c = peek_next_character(lexer);
    while (!is_newline(c))
    {
        consume_character(lexer);
        c = peek_next_character(lexer);
    }
}

static Token make_one_character_token(Lexer* lexer, TokenKind type)
{
    Token token;
    token.type = type;
    token.pos = lexer->cursor;
    consume_character(lexer);
    return token;
}

static Token make_number(Lexer* lexer)
{
    Token token;
    token.type = Token_Number;
    token.pos = lexer->cursor;

    u8 c = peek_next_character(lexer);

    i32 sign = 1;
    if (c == '-' || c == '+')
    {
        sign = (c == '+') - (c == '-');

        consume_character(lexer);
        c = peek_next_character(lexer);
    }
    else if (c == '0')
    {
        consume_character(lexer);
        c = peek_next_character(lexer);

        if (c == 'x' || c == 'X')
        {
            consume_character(lexer);
            c = peek_next_character(lexer);

            u64 num_of_digits = 0;
            u64 hex_value = 0;
            while (is_hexadecimal(c))
            {
                u8 hex_digit = 0;

                if (is_number(c))
                    hex_digit = c - '0';
                else if (c >= 'A' && c <= 'F')
                    hex_digit = 10 + c - 'A';
                else if (c >= 'a' && c <= 'f')
                    hex_digit = 10 + c - 'a';

                hex_value = (hex_value << 4) + hex_digit;

                if (num_of_digits == 16)
                    report_parsing_error(lexer, &token, "Hex number is too long (16 character maximum.)");

                num_of_digits += 1;

                consume_character(lexer);
                c = peek_next_character(lexer);
            }

            token.uint_value = hex_value;
            token.value_flags |= ValueFlag_Hex;

            if (num_of_digits == 0)
                report_parsing_error(lexer, &token, "Expected some hexadecimal characters after \"0x\", but found none.");

            if (num_of_digits > 16)
                token.value_flags |= ValueFlag_RequiresFloat64;

            return token;
        }
    }

    i64 int_part = 0;
    while (is_number(c))
    {
        u8 digit = c - '0';
        int_part = 10 * int_part + digit;

        consume_character(lexer);
        c = peek_next_character(lexer);
    }

    if (c == '.')
    {
        consume_character(lexer);
        c = peek_next_character(lexer);

        f64 float_part = 0.0;
        f64 running_10s = 0.1f;
        u64 significant_figures = 0;
        while (is_number(c))
        {
            u8 digit = c - '0';
            float_part += digit * running_10s;
            running_10s /= 10.f;

            significant_figures += 1;

            consume_character(lexer);
            c = peek_next_character(lexer);
        }

        // @HACK(dubgron): This is just an approximation.
        if (significant_figures > 8)
        {
            token.float64_value = (int_part + float_part) * sign;
            token.value_flags |= ValueFlag_RequiresFloat64;
        }
        else
        {
            token.float32_value = (int_part + float_part) * sign;
        }

        token.value_flags |= ValueFlag_Float;
    }
    else
    {
        token.int_value = int_part * sign;
    }

    return token;
}

static Token make_string(Lexer* lexer)
{
    Token result;
    result.type = Token_String;
    result.pos = lexer->cursor;

    consume_character(lexer);

    String string;
    string.data = lexer->buffer.data + lexer->cursor;

    u8 c = peek_next_character(lexer);
    while (c != '"')
    {
        consume_character(lexer);
        string.length += 1;

        c = peek_next_character(lexer);
    }

    consume_character(lexer);

    result.string_value = string;
    return result;
}

static Token make_identifier_or_keyword(Lexer* lexer)
{
    Token result;
    result.pos = lexer->cursor;

    String string;
    string.data = lexer->buffer.data + lexer->cursor;

    u8 c = peek_next_character(lexer);
    while (can_continue_identifier(c))
    {
        consume_character(lexer);
        string.length += 1;

        c = peek_next_character(lexer);
    }

    if (string == "true")
    {
        result.type = Token_Keyword_True;
        result.bool_value = true;
    }
    else if (string == "false")
    {
        result.type = Token_Keyword_False;
        result.bool_value = false;
    }
    else
    {
        result.type = Token_Identifier;
        result.string_value = string;
    }

    return result;
}

static Token make_new_token(Lexer* lexer)
{
    consume_whitespace(lexer);

    u8 c = peek_next_character(lexer);

    if (c == '\0')
    {
        return make_one_character_token(lexer, Token_EndOfFile);
    }

    if (can_start_identifier(c))
    {
        return make_identifier_or_keyword(lexer);
    }

    if (can_start_number(c))
    {
        return make_number(lexer);
    }

    if (c == '"')
    {
        return make_string(lexer);
    }

    if (c == ';')
    {
        Token token;
        token.type = Token_Comment;
        token.pos = lexer->cursor + 1;

        consume_until_new_line(lexer);

        return token;
    }

    return make_one_character_token(lexer, (TokenKind)c);
}

static Token* peek_next_token(Lexer* lexer)
{
    if (!lexer->last_token_available)
    {
        lexer->last_token = make_new_token(lexer);
        lexer->last_token_available = true;
    }

    return &lexer->last_token;
}

static void consume_token(Lexer* lexer)
{
    lexer->last_token_available = false;
}

//////////////////////////////////////////////////

static void consume_comments(Lexer* lexer)
{
    Token* token = peek_next_token(lexer);
    while (token->type == Token_Comment)
    {
        consume_token(lexer);
        token = peek_next_token(lexer);
    }
}

static ParseTreeNode* make_parse_tree_node(MemoryArena* arena, ParseTreeNode* parent)
{
    if (parent->child_count > 0)
    {
        parent->child_last->next = arena_push<ParseTreeNode>(arena);
        parent->child_last->next->prev = parent->child_last;
        parent->child_last = parent->child_last->next;
    }
    else
    {
        parent->child_first = arena_push<ParseTreeNode>(arena);
        parent->child_last = parent->child_first;
    }

    parent->child_count += 1;
    parent->child_last->parent = parent;

    return parent->child_last;
}

static ParseTreeNode* make_node_into_array(MemoryArena* arena, ParseTreeNode* parent)
{
    parent->type = ParseTreeNode_ArrayOfStructs;

    ParseTreeNode* element = arena_push<ParseTreeNode>(arena);
    element->type = ParseTreeNode_Struct;

    for (ParseTreeNode* child = parent->child_first; child; child = child->next)
    {
        child->parent = element;
    }

    element->child_first = parent->child_first;
    element->child_last = parent->child_last;
    element->child_count = parent->child_count;

    parent->child_first = parent->child_last = element;
    parent->child_count = 1;

    element->parent = parent;

    return element;
}

static bool is_literal(TokenKind token_type)
{
    switch (token_type)
    {
        case Token_Number:
        case Token_String:
        case Token_Keyword_True:
        case Token_Keyword_False:
            return true;

        default:
            return false;
    }
}

static void parse_literals(MemoryArena* arena, Lexer* lexer, ParseTreeNode* parent)
{
    Token* token = peek_next_token(lexer);
    TokenKind type_of_literals = token->type;

    while (token->type == type_of_literals)
    {
        ParseTreeNode* node = make_parse_tree_node(arena, parent);

        if (token->type == Token_Number)
        {
            node->type = ParseTreeNode_Number;

            if (token->value_flags & ValueFlag_Float)
            {
                node->float64_value = token->float64_value;
                node->value_flags |= ValueFlag_Float;
            }
            else if (token->value_flags & ValueFlag_Hex)
            {
                node->uint_value = token->uint_value;
                node->value_flags |= ValueFlag_Hex;
            }
            else
            {
                node->int_value = token->int_value;
            }

            if (token->value_flags & ValueFlag_RequiresFloat64)
                node->value_flags |= ValueFlag_RequiresFloat64;
        }
        else if (token->type == Token_String)
        {
            node->type = ParseTreeNode_String;
            node->string_value = push_string(arena, token->string_value);
        }
        else if (token->type == Token_Keyword_True)
        {
            node->type = ParseTreeNode_Boolean;
            node->bool_value = true;
        }
        else if (token->type == Token_Keyword_False)
        {
            node->type = ParseTreeNode_Boolean;
            node->bool_value = false;
        }
        else
        {
            APORIA_UNREACHABLE();
        }

        consume_token(lexer);
        consume_comments(lexer);

        token = peek_next_token(lexer);
    }

    if (is_literal(token->type))
    {
        report_parsing_error(lexer, "Field can't have values of multiple type.");

        switch (type_of_literals)
        {
            case Token_Number: report_detail(lexer, "Number expected.");   break;
            case Token_String: report_detail(lexer, "String expected.");     break;

            case Token_Keyword_True:
            case Token_Keyword_False:
                report_detail(lexer, "True or false expected."); break;
        }

        return;
    }
}

static void parse_field(MemoryArena* arena, Lexer* lexer, ParseTreeNode* parent);

static void parse_struct(MemoryArena* arena, Lexer* lexer, ParseTreeNode* parent)
{
    Token* token = peek_next_token(lexer);
    APORIA_ASSERT(token->type == Token_StructBegin);

    while (token->type == Token_StructBegin)
    {
        ParseTreeNode* node = parent;

        if (parent->child_count > 0)
        {
            if (parent->type != ParseTreeNode_ArrayOfStructs)
            {
                make_node_into_array(arena, parent);
            }

            node = make_parse_tree_node(arena, parent);
        }

        node->type = ParseTreeNode_Struct;

        consume_token(lexer);
        consume_comments(lexer);

        i32 parsed_fields_count = 0;

        token = peek_next_token(lexer);
        while (token->type == Token_Identifier)
        {
            parse_field(arena, lexer, node);
            token = peek_next_token(lexer);

            parsed_fields_count += 1;
        }

        if (token->type != Token_StructEnd)
        {
            report_parsing_error(lexer, "Closing brace expected.");
        }
        else if (parsed_fields_count == 0)
        {
            report_parsing_error(lexer, "Struct doesn't have any fields.");
        }

        consume_token(lexer);

        token = peek_next_token(lexer);
    }
}

static void parse_field(MemoryArena* arena, Lexer* lexer, ParseTreeNode* parent)
{
    Token* token = peek_next_token(lexer);
    APORIA_ASSERT(token->type == Token_Identifier);

    ParseTreeNode* node = make_parse_tree_node(arena, parent);
    node->type = ParseTreeNode_Field;
    node->name = push_string(arena, token->string_value);

    consume_token(lexer);
    consume_comments(lexer);

    token = peek_next_token(lexer);

    if (is_literal(token->type))
    {
        parse_literals(arena, lexer, node);
    }
    else if (token->type == Token_StructBegin)
    {
        parse_struct(arena, lexer, node);
    }
    else if (token->type == Token_Identifier)
    {
        report_parsing_error(lexer, "Fields can't refer to other fields... at least not yet!");
    }
    else if (token->type == '.')
    {
        report_parsing_error(lexer, "Period character is dissalowed in the name of the field.");
    }
    else
    {
        report_parsing_error(lexer, "Literal or opening brace expected after the field declaration.");
    }
}

static void parse_category(MemoryArena* arena, Lexer* lexer, ParseTreeNode* parent)
{
    consume_token(lexer);
    consume_comments(lexer);

    Token* token = peek_next_token(lexer);

    if (token->type != Token_Identifier)
    {
        report_parsing_error(lexer, "Identifier expected.");
        return;
    }

    ParseTreeNode* node = make_parse_tree_node(arena, parent);
    node->type = ParseTreeNode_Category;
    node->name = push_string(arena, token->string_value);

    consume_token(lexer);
    consume_comments(lexer);

    token = peek_next_token(lexer);

    if (token->type != Token_CategoryEnd)
    {
        report_parsing_error(lexer, "Closing brace expected.");
        return;
    }

    consume_token(lexer);
}

static ParseTreeNode* parse(MemoryArena* arena, Lexer* lexer)
{
    ParseTreeNode* parse_tree = arena_push<ParseTreeNode>(arena);
    parse_tree->type = ParseTreeNode_Root;

    Token* token = peek_next_token(lexer);
    while (token->type != Token_EndOfFile)
    {
        ParseTreeNode* root_or_last_category = parse_tree;
        if (parse_tree->child_count > 0 && parse_tree->child_last->type == ParseTreeNode_Category)
            root_or_last_category = parse_tree->child_last;

        if (token->type == Token_Comment)
        {
            consume_token(lexer);
        }
        else if (token->type == Token_CategoryBegin)
        {
            parse_category(arena, lexer, parse_tree);
        }
        else if (token->type == Token_Identifier)
        {
            parse_field(arena, lexer, root_or_last_category);
        }
        else
        {
            report_parsing_error(lexer, "Unable to parse this statement.");
        }

        if (lexer->error_occured)
            return nullptr;

        token = peek_next_token(lexer);
    }

    return parse_tree;
}

ParseTreeNode* parse_from_memory(MemoryArena* arena, String contents)
{
    Lexer lexer;
    lexer.buffer = contents;
    lexer.source_filepath = "string";

    return parse(arena, &lexer);
}

ParseTreeNode* parse_from_file(MemoryArena* arena, String filepath)
{
    ScratchArena temp = scratch_begin(arena);
    defer { scratch_end(temp); };

    Lexer lexer;
    lexer.buffer = read_entire_file(temp.arena, filepath);
    lexer.source_filepath = filepath;
    ParseTreeNode* result = parse(arena, &lexer);

    return result;
}

static String make_indent(MemoryArena* arena, i32 depth)
{
    i32 indent_size = 4;
    String result = push_string(arena, indent_size * depth);
    for (i32 idx = 0; idx < result.length; ++idx)
    {
        result.data[idx] = ' ';
    }
    return result;
}

void print_parse_tree(ParseTreeNode* node, i32 depth /* = -1 */)
{
    ScratchArena temp = scratch_begin();
    defer { scratch_end(temp); };

    String indent = depth > 0 ? make_indent(temp.arena, depth) : "";

    switch (node->type)
    {
        case ParseTreeNode_Category:        APORIA_LOG(Debug, "% [%]", indent, node->name); break;
        case ParseTreeNode_Field:           APORIA_LOG(Debug, "% %", indent, node->name); break;
        case ParseTreeNode_Struct:          APORIA_LOG(Debug, "% %", indent, node->name); break;
        case ParseTreeNode_ArrayOfStructs:  APORIA_LOG(Debug, "% %", indent, node->name); break;

        case ParseTreeNode_Number:
        {
            if (node->value_flags & ValueFlag_Float)
                if (node->value_flags * ValueFlag_RequiresFloat64)
                    APORIA_LOG(Debug, "% %", indent, node->float64_value);
                else
                    APORIA_LOG(Debug, "% %", indent, node->float32_value);
            if (node->value_flags & ValueFlag_Hex)
                APORIA_LOG(Debug, "% %", indent, node->uint_value);
            else
                APORIA_LOG(Debug, "% %", indent, node->int_value);
        }
        break;

        case ParseTreeNode_String:          APORIA_LOG(Debug, "% \"%\"", indent, node->string_value); break;
        case ParseTreeNode_Boolean:         APORIA_LOG(Debug, "% %", indent, node->bool_value ? "true" : "false"); break;
    }

    bool wrap_in_braces = (node->type == ParseTreeNode_Struct);

    if (wrap_in_braces)
        APORIA_LOG(Debug, "% {", indent);

    if (node->child_count > 0)
    {
        print_parse_tree(node->child_first, depth + 1);
    }

    if (wrap_in_braces)
        APORIA_LOG(Debug, "% }", indent);

    if (node->next)
    {
        print_parse_tree(node->next, depth);
    }
}
