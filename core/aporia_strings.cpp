#include "aporia_strings.hpp"

#include "aporia_debug.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    void String::clear()
    {
        data = nullptr;
        length = 0;
    }

    String String::substr(u64 offset, u64 count) const
    {
        APORIA_ASSERT(data);
        APORIA_ASSERT(length > offset);
        return String{ data + offset, clamp<u64>(count, 0, length - offset) };
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

    bool String::contains(String substring) const
    {
        if (substring.length > length)
        {
            return false;
        }

        const u64 substr_num = length - substring.length + 1;
        for (u64 offset = 0; offset < substr_num; ++offset)
        {
            const String temp = substr(offset, substring.length);
            if (substring == temp)
            {
                return true;
            }
        }

        return false;
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
        return *this == create_string(string);
    }

    String create_string(const char* string)
    {
        return String{ (u8*)string, strlen(string) };
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
            result.data = arena->push<u8>(length);
        }
        return result;
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

    String StringList::join(MemoryArena* arena, String delim) const
    {
        if (node_count == 0)
        {
            return String{};
        }

        const u64 delim_length = (node_count - 1) * delim.length;
        const u64 joined_length = total_length + delim_length;
        const String result = push_string(arena, joined_length);

        u64 offset = 0;
        for (const StringNode* node = first; node; node = node->next)
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
