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
