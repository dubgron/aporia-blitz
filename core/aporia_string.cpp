#include "aporia_string.hpp"

#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>

#include "aporia_debug.hpp"
#include "aporia_game.hpp"
#include "aporia_utils.hpp"

bool String::is_empty() const
{
    return length == 0;
}

String String::substr(u64 offset, u64 count /* = -1 */) const
{
    APORIA_ASSERT(length >= offset);
    return String{ data + offset, min<u64>(count, length - offset) };
}

String String::trim(u8 character /* = ' ' */) const
{
    return trim_left(character).trim_right(character);
}

String String::trim_left(u8 character /* = ' ' */) const
{
    String result{ data, length };
    while (result.length > 0 && *result.data == character)
    {
        result.data += 1;
        result.length -= 1;
    }
    return result;
}

String String::trim_right(u8 character /* = ' ' */) const
{
    String result{ data, length };
    while (result.length > 0 && result.data[result.length - 1] == character)
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
                String string = substr(offset, idx - offset);
                result.push_node(arena, string);
            }
            offset = idx + 1;
        }
    }

    if (length > offset)
    {
        String string = substr(offset);
        result.push_node(arena, string);
    }

    return result;
}

u64 String::find(u8 character, u64 offset /* = 0 */) const
{
    if (offset >= length)
    {
        return INDEX_INVALID;
    }

    for (u64 off = offset; off < length; ++off)
    {
        if (data[off] == character)
        {
            return off;
        }
    }

    return INDEX_INVALID;
}

u64 String::find(String other, u64 offset /* = 0 */) const
{
    if (other.length + offset > length)
    {
        return INDEX_INVALID;
    }

    u64 substr_num = length - other.length + 1;
    for (u64 off = offset; off < substr_num; ++off)
    {
        String temp = substr(off, other.length);
        if (other == temp)
        {
            return off;
        }
    }

    return INDEX_INVALID;
}

u64 String::find_eol(u64 offset /* = 0 */) const
{
    if (offset >= length)
    {
        return INDEX_INVALID;
    }

    for (u64 off = offset; off < length; ++off)
    {
        if (data[off] == '\n')
        {
            return off;
        }
    }

    return INDEX_INVALID;
}

u64 String::rfind(u8 character, u64 offset /* = -1 */) const
{
    offset = min(offset, length - 1);
    for (i64 off = offset; off >= 0; --off)
    {
        if (data[off] == character)
        {
            return off;
        }
    }
    return INDEX_INVALID;
}

u64 String::rfind(String other, u64 offset /* = -1 */) const
{
    if (length < other.length || other.length == 0)
    {
        return INDEX_INVALID;
    }

    // There is no point in starting the search at offset greater than
    // this.length - other.length as the other string would be longer
    // than the substring between offset and this.length - 1.
    offset = min(offset, length - other.length);

    for (i64 off = offset; off >= 0; --off)
    {
        String temp = substr(off, other.length);
        if (temp == other)
        {
            return off;
        }
    }

    return INDEX_INVALID;
}

bool String::contains(String other) const
{
    return find(other) != INDEX_INVALID;
}

bool String::starts_with(String other) const
{
    String temp = substr(0, other.length);
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

bool String::operator==(CString other) const
{
    return *this == String{ other };
}

CString String::cstring(MemoryArena* arena) const
{
    String result = push_string(arena, length + 1);
    memcpy(result.data, data, length * sizeof(u8));
    result.data[length] = '\0';
    return reinterpret_cast<CString>(result.data);
}

CString String::operator*() const
{
    return cstring(&memory.frame);
}

i32 string_compare(String str0, String str1)
{
    u64 min_length = min(str0.length, str1.length);
    for (u64 idx = 0; idx < min_length; ++idx)
    {
        if (str0.data[idx] > str1.data[idx]) return 1;
        if (str0.data[idx] < str1.data[idx]) return -1;
    }

    if (str0.length > str1.length) return 1;
    if (str0.length < str1.length) return -1;

    return 0;
}

i32 string_compare(const String* str0, const String* str1)
{
    return string_compare(*str0, *str1);
}

String push_string(MemoryArena* arena, String string)
{
    String result = push_string(arena, string.length);
    memcpy(result.data, string.data, string.length * sizeof(u8));
    return result;
}

String push_string(MemoryArena* arena, CString string)
{
    u64 length = strlen(string);
    String result = push_string(arena, length);
    memcpy(result.data, string, length * sizeof(u8));
    return result;
}

String push_string(MemoryArena* arena, u64 length)
{
    String result;
    if (length > 0)
    {
        result.data = arena_push_uninitialized<u8>(arena, length);
        result.length = length;
    }
    return result;
}

String string_concat(MemoryArena* arena, String first, String second)
{
    String result = push_string(arena, first.length + second.length);
    memcpy(result.data, first.data, first.length);
    memcpy(result.data + first.length, second.data, second.length);
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

    ScratchArena temp = scratch_begin();
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
    scratch_end(temp);

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
    StringNode* node = arena_push<StringNode>(arena);
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
    StringNode* node = arena_push<StringNode>(arena);
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

    u64 delim_length = (node_count - 1) * delim.length;
    u64 joined_length = total_length + delim_length;
    String result = push_string(arena, joined_length);

    u64 offset = 0;
    for (StringNode* node = first; node && node->prev != last; node = node->next)
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

template<typename... Ts>
static String sprintf_cstyle(MemoryArena* arena, String format, Ts&&... args)
{
    char log_buffer[1024];
    stbsp_sprintf(log_buffer, *format, std::forward<Ts>(args)...);
    return push_string(arena, log_buffer);
}

String to_hex(MemoryArena* arena, i64 value)
{
    return sprintf_cstyle(arena, "0x%016x", value);
}

String to_hex(MemoryArena* arena, u64 value)
{
    return sprintf_cstyle(arena, "0x%016x", value);
}

String to_hex(MemoryArena* arena, u32 value)
{
    return sprintf_cstyle(arena, "0x%08x", value);
}

String to_hex(MemoryArena* arena, f32 value)
{
    return to_hex(arena, *(u32*)&value);
}

String to_string(MemoryArena* arena, i64 value)
{
    return sprintf_cstyle(arena, "%lld", value);
}

String to_string(MemoryArena* arena, i32 value)
{
    return sprintf_cstyle(arena, "%d", value);
}

String to_string(MemoryArena* arena, i16 value)
{
    return sprintf_cstyle(arena, "%hd", value);
}

String to_string(MemoryArena* arena, i8 value)
{
    return sprintf_cstyle(arena, "%c", value);
}

String to_string(MemoryArena* arena, u64 value)
{
    return sprintf_cstyle(arena, "%llu", value);
}

String to_string(MemoryArena* arena, u32 value)
{
    return sprintf_cstyle(arena, "%u", value);
}

String to_string(MemoryArena* arena, u16 value)
{
    return sprintf_cstyle(arena, "%hu", value);
}

String to_string(MemoryArena* arena, u8 value)
{
    return sprintf_cstyle(arena, "%c", value);
}

String to_string(MemoryArena* arena, f64 value)
{
    return sprintf_cstyle(arena, "%f", value);
}

String to_string(MemoryArena* arena, f32 value)
{
    return sprintf_cstyle(arena, "%f", value);
}

String to_string(MemoryArena* arena, const v4& value)
{
    return sprintf_cstyle(arena, "(%f, %f, %f, %f)", value.x, value.y, value.z, value.w);
}

String to_string(MemoryArena* arena, const v3& value)
{
    return sprintf_cstyle(arena, "(%f, %f, %f)", value.x, value.y, value.z);
}

String to_string(MemoryArena* arena, const v2& value)
{
    return sprintf_cstyle(arena, "(%f, %f)", value.x, value.y);
}

String to_string(MemoryArena* arena, const v4_i32& value)
{
    return sprintf_cstyle(arena, "(%d, %d, %d, %d)", value.x, value.y, value.z, value.w);
}

String to_string(MemoryArena* arena, const v3_i32& value)
{
    return sprintf_cstyle(arena, "(%d, %d, %d)", value.x, value.y, value.z);
}

String to_string(MemoryArena* arena, const v2_i32& value)
{
    return sprintf_cstyle(arena, "(%d, %d)", value.x, value.y);
}

String to_string(MemoryArena* arena, const v4_u32& value)
{
    return sprintf_cstyle(arena, "(%u, %u, %u, %u)", value.x, value.y, value.z, value.w);
}

String to_string(MemoryArena* arena, const v3_u32& value)
{
    return sprintf_cstyle(arena, "(%u, %u, %u)", value.x, value.y, value.z);
}

String to_string(MemoryArena* arena, const v2_u32& value)
{
    return sprintf_cstyle(arena, "(%u, %u)", value.x, value.y);
}

String to_string(MemoryArena* arena, const m4& value)
{
    return sprintf_cstyle(arena, "( (%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f), (%f, %f, %f, %f) )",
        value[0][0], value[0][1], value[0][2], value[0][3],
        value[1][0], value[1][1], value[1][2], value[1][3],
        value[2][0], value[2][1], value[2][2], value[2][3],
        value[3][0], value[3][1], value[3][2], value[3][3]);
}

String to_string(MemoryArena* arena, const m3& value)
{
    return sprintf_cstyle(arena, "( (%f, %f, %f), (%f, %f, %f), (%f, %f, %f) )",
        value[0][0], value[0][1], value[0][2],
        value[1][0], value[1][1], value[1][2],
        value[2][0], value[2][1], value[2][2]);
}

String to_string(MemoryArena* arena, const m2& value)
{
    return sprintf_cstyle(arena, "( (%f, %f), (%f, %f) )",
        value[0][0], value[0][1],
        value[1][0], value[1][1]);
}

String to_string(MemoryArena* arena, bool value)
{
    return push_string(arena, value ? "true" : "false");
}

String to_string(MemoryArena* arena, String value)
{
    return value;
}

String to_string(MemoryArena* arena, CString value)
{
    return push_string(arena, value);
}
