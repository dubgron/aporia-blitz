#pragma once

#include <string.h>
#include <string_view>

#include "aporia_memory.hpp"
#include "aporia_types.hpp"

namespace Aporia
{
    struct StringList;

    struct String
    {
        u8* data = nullptr;
        u64 length = 0;

        bool is_valid() const;
        void clear();

        String substr(u64 offset, u64 count = -1) const;
        String trim() const;
        String trim_left() const;
        String trim_right() const;

        String append(MemoryArena* arena, String string) const;
        String append_front(MemoryArena* arena, String string) const;

        StringList split(MemoryArena* arena, u8 delim) const;

        u64 find(u8 character, u64 offset = 0) const;
        u64 find(String substring, u64 offset = 0) const;
        u64 find_eol(u64 offset = 0) const;

        bool contains(String substring) const;
        bool starts_with(String substring) const;

        bool operator==(String string) const;
        bool operator==(const char* string) const;

        // @TODO(dubgron): Remove this in the future once we replace all usages of std.
        // @NOTE(dubgron): Currently we use std::string only in logger, and std::string_view only in json.
        operator std::string_view() const { return std::string_view{ reinterpret_cast<char*>(data), static_cast<size_t>(length) }; }
        const char* operator*() const { return reinterpret_cast<const char*>(data); }

        // @HACK(dubgron): Cringe constructors to handle conversion from C-strings and std::string_views.
        String() = default;
        String(u8* data, u64 length) : data(data), length(length) {}
        String(const char* string) : data( (u8*)string ), length( strlen(string) ) {}
        String(std::string_view string) : data( (u8*)string.data() ), length( string.length() ) {}

        static constexpr u64 INVALID_INDEX = -1;
    };

    String push_string(MemoryArena* arena, String string);
    String push_string(MemoryArena* arena, const char* string);
    String push_string(MemoryArena* arena, u64 length);

    i64 string_to_int(String string);
    f32 string_to_float(String string);
    bool string_to_bool(String string);

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

        StringList pop_node() const;
        StringList pop_node_front() const;

        StringNode* get_node_at_index(u64 index) const;

        String join(MemoryArena* arena, String delim = "") const;
    };
}

// @TODO(dubgron): Remove this in the future once we replace all usages of std.
template <>
struct std::hash<Aporia::String>
{
    std::size_t operator()(const Aporia::String& str) const
    {
        const string_view sv{ (char*)str.data, (size_t)str.length };
        return hash<string_view>()(sv);
    }
};
