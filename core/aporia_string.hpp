#pragma once

#include <string.h>

#include "aporia_game.hpp"
#include "aporia_memory.hpp"
#include "aporia_types.hpp"

struct StringList;

using CString = const char*;

struct String
{
    u8* data = nullptr;
    u64 length = 0;

    bool is_empty() const;

    [[nodiscard]] String substr(u64 offset, u64 count = -1) const;
    String trim(u8 character = ' ') const;
    String trim_left(u8 character = ' ') const;
    String trim_right(u8 character = ' ') const;

    StringList split(MemoryArena* arena, u8 delim) const;

    u64 find(u8 character, u64 offset = 0) const;
    u64 find(String other, u64 offset = 0) const;
    u64 find_eol(u64 offset = 0) const;

    u64 rfind(u8 character, u64 offset = -1) const;
    u64 rfind(String other, u64 offset = -1) const;

    bool contains(String other) const;
    bool starts_with(String other) const;

    bool operator==(String other) const;
    bool operator==(CString other) const;

    String() = default;
    String(u8* data, u64 length) : data(data), length(length) {}

    // @HACK(dubgron): Cringe constructors and operators to handle conversion between Strings and C-strings.
    String(CString string) : data((u8*)string), length(string ? strlen(string) : 0) {}
    CString cstring(MemoryArena* arena) const;
    CString operator*() const; // @NOTE(dubgron): This allocates a temporary null-terminated string on the frame arena.
};

i32 string_compare(String str0, String str1);
i32 string_compare(const String* str0, const String* str1);

struct StringNode
{
    StringNode* next = nullptr;
    StringNode* prev = nullptr;
    String string;
};

struct StringList
{
    StringNode* first = nullptr;
    StringNode* last = nullptr;
    u64 node_count = 0;
    u64 total_length = 0;

    void clear();

    void push_node(MemoryArena* arena, String string);
    void push_node_front(MemoryArena* arena, String string);

    void append(StringList other);

    StringList pop_node() const;
    StringList pop_node_front() const;

    StringNode* get_node_at_index(u64 index) const;

    String join(MemoryArena* arena, String delim = "") const;
};

String push_string(MemoryArena* arena, String string);
String push_string(MemoryArena* arena, CString string);
String push_string(MemoryArena* arena, u64 length);

String string_concat(MemoryArena* arena, String first, String second);

i64 string_to_int(String string);
f32 string_to_float(String string);
bool string_to_bool(String string);

String to_hex(MemoryArena* arena, i64 value);
String to_hex(MemoryArena* arena, u64 value);
String to_hex(MemoryArena* arena, u32 value);
String to_hex(MemoryArena* arena, f32 value);

String to_string(MemoryArena* arena, i64 value);
String to_string(MemoryArena* arena, i32 value);
String to_string(MemoryArena* arena, i16 value);
String to_string(MemoryArena* arena, i8 value);

String to_string(MemoryArena* arena, u64 value);
String to_string(MemoryArena* arena, u32 value);
String to_string(MemoryArena* arena, u16 value);
String to_string(MemoryArena* arena, u8 value);

String to_string(MemoryArena* arena, f64 value);
String to_string(MemoryArena* arena, f32 value);

String to_string(MemoryArena* arena, const v4& value);
String to_string(MemoryArena* arena, const v3& value);
String to_string(MemoryArena* arena, const v2& value);

String to_string(MemoryArena* arena, const v4_i32& value);
String to_string(MemoryArena* arena, const v3_i32& value);
String to_string(MemoryArena* arena, const v2_i32& value);

String to_string(MemoryArena* arena, const v4_u32& value);
String to_string(MemoryArena* arena, const v3_u32& value);
String to_string(MemoryArena* arena, const v2_u32& value);

String to_string(MemoryArena* arena, const m4& value);
String to_string(MemoryArena* arena, const m3& value);
String to_string(MemoryArena* arena, const m2& value);

String to_string(MemoryArena* arena, bool value);

template<typename E> requires std::is_enum_v<E>
String to_string(MemoryArena* arena, E value)
{
    return to_string(arena, to_underlying(value));
}

String to_string(MemoryArena* arena, String value);
String to_string(MemoryArena* arena, CString value);

template<typename T, typename... Ts>
[[nodiscard]] String sprintf(MemoryArena* arena, String format, T arg, Ts... args)
{
    StringList builder;
    ScratchArena temp = scratch_begin(arena);

    u64 args_begin = format.find('%');
    while (args_begin < format.length - 1 && format.data[args_begin + 1] == '%')
    {
        args_begin = format.find('%', args_begin + 2);
    }

    String before_args = format.substr(0, args_begin);
    if (before_args.length > 0)
    {
        builder.push_node(temp.arena, before_args);
    }

    if (args_begin < format.length)
    {
        String arg_as_string = to_string(temp.arena, arg);
        builder.push_node(temp.arena, arg_as_string);

        String after_args = format.substr(args_begin + 1);
        if (after_args.length > 0)
        {
            if constexpr (sizeof...(args) > 0)
            {
                after_args = sprintf(temp.arena, after_args, std::forward<Ts>(args)...);
            }

            builder.push_node(temp.arena, after_args);
        }
    }

    String result = builder.join(arena);
    scratch_end(temp);

    return result;
}

template<typename T, typename... Ts>
[[nodiscard]] String tprintf(String format, T arg, Ts... args)
{
    return sprintf(&memory.frame, format, arg, args...);
}
