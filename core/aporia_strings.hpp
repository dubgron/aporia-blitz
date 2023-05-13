#pragma once

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

        void clear();

        String substr(u64 offset, u64 count = -1) const;
        String trim() const;
        String trim_left() const;
        String trim_right() const;
        String append(MemoryArena* arena, String string) const;

        StringList split(MemoryArena* arena, u8 delim) const;

        bool contains(String substring) const;

        bool operator==(String string) const;
        bool operator==(const char* string) const;

        // @TODO(dubgron): Remove this in the future once we replace all usages of std.
        operator std::string_view() const { return std::string_view{ reinterpret_cast<char*>(data), static_cast<size_t>(length) }; }
        const char* to_cstring(MemoryArena* arena) const;

        i64 to_int() const;
        f32 to_float() const;
        bool to_bool() const;
    };

    String create_string(const char* string);
    String push_string(MemoryArena* arena, const char* string);
    String push_string(MemoryArena* arena, u64 length);

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

        String join(MemoryArena* arena, String delim) const;
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
