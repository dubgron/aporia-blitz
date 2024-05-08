#pragma once

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

    ParseTreeNode_Integer,
    ParseTreeNode_Float,
    ParseTreeNode_String,
    ParseTreeNode_Boolean,
};

struct ParseTreeNode
{
    ParseTreeNodeType type = ParseTreeNode_Invalid;

    union
    {
        String name;

        i64 int_value = 0;
        f64 float_value;
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
    APORIA_ASSERT(node->type == ParseTreeNode_Field);
    ParseTreeNode* child = node->child_first;
    APORIA_ASSERT(node->child_count == 1 && child->type == ParseTreeNode_Integer);
    *out_value = child->int_value;
}

template<typename T> requires std::is_floating_point_v<T>
void get_value_from_node(ParseTreeNode* node, T* out_value)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field);
    ParseTreeNode* child = node->child_first;
    APORIA_ASSERT(node->child_count == 1 && child->type == ParseTreeNode_Float);
    *out_value = child->float_value;
}

void get_value_from_node(ParseTreeNode* node, String* out_value)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field);
    ParseTreeNode* child = node->child_first;
    APORIA_ASSERT(node->child_count == 1 && child->type == ParseTreeNode_String);
    *out_value = child->string_value;
}

void get_value_from_node(ParseTreeNode* node, bool* out_value)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field);
    ParseTreeNode* child = node->child_first;
    APORIA_ASSERT(node->child_count == 1 && child->type == ParseTreeNode_Boolean);
    *out_value = child->bool_value;
}

template<typename T> requires std::is_integral_v<T>
void get_value_from_node(ParseTreeNode* node, T* out_array, i64 count)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field && node->child_count == count);
    ParseTreeNode* child = node->child_first;
    for (i64 idx = 0; idx < node->child_count; ++idx)
    {
        APORIA_ASSERT(child->type == ParseTreeNode_Integer);
        out_array[idx] = child->int_value;
        child = child->next;
    }
}

template<typename T> requires std::is_floating_point_v<T>
void get_value_from_node(ParseTreeNode* node, T* out_array, i64 count)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field && node->child_count == count);
    ParseTreeNode* child = node->child_first;
    for (i64 idx = 0; idx < node->child_count; ++idx)
    {
        APORIA_ASSERT(child->type == ParseTreeNode_Float);
        out_array[idx] = child->float_value;
        child = child->next;
    }
}

void get_value_from_node(ParseTreeNode* node, String* out_array, i64 count)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field && node->child_count == count);
    ParseTreeNode* child = node->child_first;
    for (i64 idx = 0; idx < node->child_count; ++idx)
    {
        APORIA_ASSERT(child->type == ParseTreeNode_String);
        out_array[idx] = child->string_value;
        child = child->next;
    }
}

void get_value_from_node(ParseTreeNode* node, bool* out_array, i64 count)
{
    APORIA_ASSERT(node->type == ParseTreeNode_Field && node->child_count == count);
    ParseTreeNode* child = node->child_first;
    for (i64 idx = 0; idx < node->child_count; ++idx)
    {
        APORIA_ASSERT(child->type == ParseTreeNode_Boolean);
        out_array[idx] = child->bool_value;
        child = child->next;
    }
}
