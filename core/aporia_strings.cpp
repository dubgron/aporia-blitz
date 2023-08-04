#include "aporia_strings.hpp"

#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    bool String::is_valid() const
    {
        return data != nullptr;
    }

    void String::clear()
    {
        data = nullptr;
        length = 0;
    }

    String String::substr(u64 offset, u64 count /* = -1 */) const
    {
        APORIA_ASSERT(data);
        APORIA_ASSERT(length >= offset);
        return String{ data + offset, min<u64>(count, length - offset) };
    }

    String String::trim() const
    {
        return trim_left().trim_right();
    }

    String String::trim_left() const
    {
        APORIA_ASSERT(data);
        String result{ data, length };
        while (result.length > 0 && *result.data == ' ')
        {
            result.data += 1;
            result.length -= 1;
        }
        return result;
    }

    String String::trim_right() const
    {
        APORIA_ASSERT(data);
        String result{ data, length };
        while (result.length > 0 && result.data[result.length - 1] == ' ')
        {
            result.length -= 1;
        }
        return result;
    }

    String String::append(MemoryArena* arena, String string) const
    {
        String result = push_string(arena, length + string.length);
        memcpy(result.data, data, length);
        memcpy(result.data + length, string.data, string.length);
        return result;
    }

    StringList String::split(MemoryArena* arena, u8 delim) const
    {
        StringList result;

        u64 offset = 0;
        for (u64 idx = 0; idx < length; ++idx)
        {
            if (data[idx] == delim)
            {
                if (idx > offset)
                {
                    const String string = substr(offset, idx - offset);
                    result.push_node(arena, string);
                }
                offset = idx + 1;
            }
        }

        if (length > offset)
        {
            const String string = substr(offset);
            result.push_node(arena, string);
        }

        return result;
    }

    u64 String::find(u8 character, u64 offset /* = 0 */) const
    {
        if (offset >= length)
        {
            return INVALID_INDEX;
        }

        APORIA_ASSERT(data);

        for (u64 off = offset; off < length; ++off)
        {
            if (data[off] == character)
            {
                return off;
            }
        }

        return INVALID_INDEX;
    }

    u64 String::find(String substring, u64 offset /* = 0 */) const
    {
        if (substring.length + offset >= length)
        {
            return INVALID_INDEX;
        }

        const u64 substr_num = length - substring.length + 1;
        for (u64 off = offset; off < substr_num; ++off)
        {
            const String temp = substr(off, substring.length);
            if (substring == temp)
            {
                return off;
            }
        }

        return INVALID_INDEX;
    }

    static bool is_eol(u8 ch)
    {
        return ch == '\r' || ch == '\n';
    }

    u64 String::find_eol(u64 offset /* = 0 */) const
    {
        if (offset >= length)
        {
            return INVALID_INDEX;
        }

        u64 eol_begin = INVALID_INDEX;
        for (u64 off = offset; off < length; ++off)
        {
            if (is_eol(data[off]))
            {
                return off;
                eol_begin = off;
                break;
            }
        }

        u64 eol_end = INVALID_INDEX;
        for (u64 off = eol_begin + 1; off < length; ++off)
        {
            if (!is_eol(data[off]))
            {
                eol_end = off - 1;
                break;
            }
        }

        return eol_end;
    }

    bool String::contains(String substring) const
    {
        return find(substring) != INVALID_INDEX;
    }

    bool String::starts_with(String substring) const
    {
        const String temp = substr(0, substring.length);
        return substring == temp;
    }

    bool String::operator==(String string) const
    {
        if (length != string.length)
        {
            return false;
        }

        for (u64 i = 0; i < length; ++i)
        {
            if (data[i] != string.data[i])
            {
                return false;
            }
        }

        return true;
    }

    bool String::operator==(const char* string) const
    {
        return *this == String{ string };
    }

    String push_string(MemoryArena* arena, String string)
    {
        const String result = push_string(arena, string.length);
        memcpy(result.data, string.data, string.length * sizeof(u8));
        return result;
    }

    String push_string(MemoryArena* arena, const char* string)
    {
        const u64 length = strlen(string);
        const String result = push_string(arena, length);
        memcpy(result.data, string, length * sizeof(u8));
        return result;
    }

    String push_string(MemoryArena* arena, u64 length)
    {
        APORIA_ASSERT(arena);
        String result;
        if (length > 0)
        {
            result.length = length;
            result.data = arena->push<u8>(length + 1);
            result.data[length] = '\0';
        }
        return result;
    }

    i64 string_to_int(String string)
    {
        if (string.length > 0)
        {
            i64 result = 0;
            i64 running_10s = 1;
            for (u64 idx = string.length - 1; idx > 0; --idx)
            {
                u8 digit = string.data[idx] - '0';
                result += digit * running_10s;
                running_10s *= 10;
            }

            if (string.data[0] == '-')
            {
                return -result;
            }
            else
            {
                u8 digit = string.data[0] - '0';
                result += digit * running_10s;
                return result;
            }
        }

        return 0;
    }

    f32 string_to_float(String string)
    {
        ScratchArena temp = create_scratch_arena(&persistent_arena);
        StringList split_number = string.split(temp.arena, '.');

        String integral_part = split_number.first->string;
        f32 result = string_to_int(integral_part);

        if (split_number.node_count == 2)
        {
            String fractional_part = split_number.last->string;

            f32 running_10s = 0.1f;
            for (u64 idx = 0; idx < fractional_part.length; ++idx)
            {
                u8 digit = fractional_part.data[idx] - '0';
                result += digit * running_10s;
                running_10s /= 10.f;
            }
        }

        return result;
    }

    bool string_to_bool(String string)
    {
        if (string == "true")
        {
            return true;
        }
        else if (string == "false")
        {
            return false;
        }
        else
        {
            APORIA_UNREACHABLE();
            return false;
        }
    }

    void StringList::clear()
    {
        first = nullptr;
        last = nullptr;
        node_count = 0;
        total_length = 0;
    }

    void StringList::push_node(MemoryArena* arena, String string)
    {
        APORIA_ASSERT(arena);
        StringNode* node = arena->push_zero<StringNode>();
        node->string = string;

        if (node_count > 0)
        {
            APORIA_ASSERT(last);
            last->next = node;
            node->prev = last;
        }
        else
        {
            APORIA_ASSERT(!first);
            first = node;
        }

        last = node;

        node_count += 1;
        total_length += string.length;
    }

    void StringList::push_node_front(MemoryArena* arena, String string)
    {
        APORIA_ASSERT(arena);
        StringNode* node = arena->push_zero<StringNode>();
        node->string = string;

        if (node_count > 0)
        {
            APORIA_ASSERT(first);
            first->prev = node;
            node->next = first;
        }
        else
        {
            last = node;
        }

        first = node;

        node_count += 1;
        total_length += string.length;
    }

    StringList StringList::pop_node() const
    {
        StringList result;
        if (node_count > 0)
        {
            result.first = first;
            result.last = last->prev;
            result.node_count = node_count - 1;
            result.total_length = total_length - last->string.length;
        }
        return result;
    }

    StringList StringList::pop_node_front() const
    {
        StringList result;
        if (node_count > 0)
        {
            result.first = first->next;
            result.last = last;
            result.node_count = node_count - 1;
            result.total_length = total_length - first->string.length;
        }
        return result;
    }

    StringNode* StringList::get_node_at_index(u64 index) const
    {
        StringNode* result = first;
        while (result && index--)
        {
            result = result->next;
        }
        return result;
    }

    String StringList::join(MemoryArena* arena, String delim /* = "" */) const
    {
        if (node_count == 0)
        {
            return String{};
        }

        const u64 delim_length = (node_count - 1) * delim.length;
        const u64 joined_length = total_length + delim_length;
        const String result = push_string(arena, joined_length);

        u64 offset = 0;
        for (const StringNode* node = first; node && node->prev != last; node = node->next)
        {
            memcpy(result.data + offset, node->string.data, node->string.length * sizeof(u8));
            offset += node->string.length;

            if (node != last)
            {
                memcpy(result.data + offset, delim.data, delim.length * sizeof(u8));
                offset += delim.length;
            }
        }

        APORIA_ASSERT(joined_length == offset);
        return result;
    }
}
