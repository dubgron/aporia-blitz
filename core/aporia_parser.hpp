#pragma once

#include "aporia_debug.hpp"
#include "aporia_memory.hpp"
#include "aporia_string.hpp"
#include "aporia_types.hpp"

enum ParseTreeNodeType : u8
{
    ParseTreeNode_Invalid,
    ParseTreeNode_Root,

    ParseTreeNode_Category,
    ParseTreeNode_Field,
    ParseTreeNode_Struct,
    ParseTreeNode_ArrayOfStructs,

    ParseTreeNode_Number,
    ParseTreeNode_String,
    ParseTreeNode_Boolean,
};

using ValueFlags = u8;
enum ValueFlag_ : ValueFlags
{
    ValueFlag_None              = 0x00,

    ValueFlag_Float             = 0x01,
    ValueFlag_Hex               = 0x02,

    ValueFlag_RequiresFloat64   = 0x10,
};

struct ParseTreeNode
{
    ParseTreeNodeType type = ParseTreeNode_Invalid;

    ValueFlags value_flags = ValueFlag_None;

    union
    {
        String name;

        i64 int_value = 0;
        u64 uint_value;
        f64 float64_value;
        f32 float32_value;
        String string_value;
        bool bool_value;
    };

    ParseTreeNode* next = nullptr;
    ParseTreeNode* prev = nullptr;

    ParseTreeNode* parent = nullptr;

    ParseTreeNode* child_first = nullptr;
    ParseTreeNode* child_last = nullptr;
    i64 child_count = 0;
};

ParseTreeNode* parse_from_memory(MemoryArena* arena, String contents);
ParseTreeNode* parse_from_file(MemoryArena* arena, String filepath);

void print_parse_tree(ParseTreeNode* node, i32 depth = -1);

template<typename T> requires std::is_integral_v<T>
void get_value_from_node(ParseTreeNode* node, T* out_value)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Number);
    if (node->value_flags & ValueFlag_Float)
        APORIA_LOG(Warning, "Convertion from float to int, possible loss of data.");

    if (node->value_flags & ValueFlag_RequiresFloat64 && sizeof(T) < sizeof(i64))
        APORIA_LOG(Warning, "Convertion from i64 to a smaller int, possible loss of data.");

    if constexpr (std::is_signed_v<T>)
        *out_value = node->int_value;
    else
        *out_value = node->uint_value;
}

template<typename T> requires std::is_floating_point_v<T>
void get_value_from_node(ParseTreeNode* node, T* out_value)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Number);
    if (node->value_flags & ValueFlag_RequiresFloat64 && sizeof(T) < sizeof(f64))
        APORIA_LOG(Warning, "Convertion from f64 to f32, possible loss of data.");

    if (node->value_flags & (ValueFlag_Float | ValueFlag_Hex))
    {
        if (node->value_flags & ValueFlag_RequiresFloat64)
            *out_value = node->float64_value;
        else
            *out_value = node->float32_value;
    }
    else
    {
        *out_value = (T)node->int_value;
    }
}

void get_value_from_node(ParseTreeNode* node, String* out_value)
{
    APORIA_ASSERT(node->type == ParseTreeNode_String);
    *out_value = node->string_value;
}

void get_value_from_node(ParseTreeNode* node, bool* out_value)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Boolean);
    *out_value = node->bool_value;
}

template<typename T>
void get_value_from_field(ParseTreeNode* node, T* out_value)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field && node->child_count == 1);
    ParseTreeNode* child = node->child_first;

    get_value_from_node(child, out_value);
}

template<typename T>
void get_value_from_field(ParseTreeNode* node, T* out_array, i64 count)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field && node->child_count == count);
    ParseTreeNode* child = node->child_first;
    for (i64 idx = 0; idx < node->child_count; ++idx)
    {
        get_value_from_node(child, &out_array[idx]);
        child = child->next;
    }
}
