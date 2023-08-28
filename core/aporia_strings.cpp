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

    String String::append(MemoryArena* arena, String other) const
    {
        String result = push_string(arena, length + other.length);
        memcpy(result.data, data, length);
        memcpy(result.data + length, other.data, other.length);
        return result;
    }

    String String::append_front(MemoryArena* arena, String other) const
    {
        return other.append(arena, *this);
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

    u64 String::find(String other, u64 offset /* = 0 */) const
    {
        if (other.length + offset >= length)
        {
            return INVALID_INDEX;
        }

        const u64 substr_num = length - other.length + 1;
        for (u64 off = offset; off < substr_num; ++off)
        {
            const String temp = substr(off, other.length);
            if (other == temp)
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

        u64 eol = INVALID_INDEX;
        for (u64 off = offset; off < length; ++off)
        {
            if (is_eol(data[off]))
            {
                eol = off;
                break;
            }
        }

        return eol;
    }

    bool String::contains(String other) const
    {
        return find(other) != INVALID_INDEX;
    }

    bool String::starts_with(String other) const
    {
        const String temp = substr(0, other.length);
        return other == temp;
    }

    bool String::operator==(String other) const
    {
        if (length != other.length)
        {
            return false;
        }

        for (u64 i = 0; i < length; ++i)
        {
            if (data[i] != other.data[i])
            {
                return false;
            }
        }

        return true;
    }

    bool String::operator==(const char* other) const
    {
        return *this == String{ other };
    }

    const char* String::operator*() const
    {
        String result = push_string(&frame_arena, length + 1);
        memcpy(result.data, data, length * sizeof(u8));
        result.data[length] = '\0';
        return reinterpret_cast<const char*>(result.data);
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
        String result;
        if (length > 0)
        {
            result.data = arena->push<u8>(length);
            result.length = length;
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
        f32 result = 0.f;

        ScratchArena temp = create_scratch_arena(&persistent_arena);
        {
            StringList split_number = string.split(temp.arena, '.');

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

            String integral_part = split_number.first->string;
            i64 result_int = string_to_int(integral_part);

            if (result_int != 0)
            {
                result += (f32)result_int;
            }
            else if (integral_part.data[0] == '-')
            {
                result = -result;
            }
        }
        rollback_scratch_arena(temp);

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
        StringNode* node = arena->push_zero<StringNode>();
        node->string = string;

        if (node_count > 0)
        {
            APORIA_ASSERT(first && last);
            last->next = node;
            node->prev = last;
        }
        else
        {
            APORIA_ASSERT(!first && !last);
            first = node;
        }

        last = node;

        node_count += 1;
        total_length += string.length;
    }

    void StringList::push_node_front(MemoryArena* arena, String string)
    {
        StringNode* node = arena->push_zero<StringNode>();
        node->string = string;

        if (node_count > 0)
        {
            APORIA_ASSERT(first && last);
            first->prev = node;
            node->next = first;
        }
        else
        {
            APORIA_ASSERT(!first && !last);
            last = node;
        }

        first = node;

        node_count += 1;
        total_length += string.length;
    }

    void StringList::append(StringList other)
    {
        if (node_count == 0)
        {
            *this = other;
        }
        else if (other.node_count > 0)
        {
            APORIA_ASSERT(first && last);
            APORIA_ASSERT(other.first && other.last);

            last->next = other.first;
            last->next->prev = last;
            last = other.last;

            node_count += other.node_count;
            total_length += other.total_length;
        }
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
