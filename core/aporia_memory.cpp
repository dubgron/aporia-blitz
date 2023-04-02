#include "aporia_memory.hpp"

#include "debug/asserts.hpp"

namespace Aporia
{
    static u64 next_aligned(u64 address, u64 align)
    {
        u64 aligned_address = address + align - 1;
        aligned_address -= aligned_address % align;
        return aligned_address;
    }

    void MemoryArena::alloc(u64 size)
    {
        memory = malloc(size);
        max = size;
        pos = 0;
        align = 8;

        APORIA_ASSERT( PTR_TO_INT(memory) % align == 0 );
    }

    void MemoryArena::dealloc()
    {
        free(memory);
        memory = nullptr;
        max = 0;
        pos = 0;
        align = 8;
    }

    void MemoryArena::clear()
    {
        pos = 0;
    }

    void* MemoryArena::push(u64 size)
    {
        const u64 space_left = max - pos;
        APORIA_VALIDATE_OR_RETURN_VALUE(size > 0 && size <= space_left, nullptr,
            "Can't allocate {} B! Pos: {} B, Max: {} B, Left: {} B", size, pos, max, space_left);

        const u64 result = PTR_TO_INT(memory) + pos;
        pos = next_aligned(pos + size, align);
        return INT_TO_PTR(result);
    }

    void* MemoryArena::push_zero(u64 size)
    {
        void* result = push(size);
        memset(result, 0, size);
        return result;
    }

    void TempArena::begin(MemoryArena* in_arena)
    {
        APORIA_VALIDATE_OR_RETURN(in_arena, "The arena is null! Can't create a TempArena!");

        arena = in_arena;
        pos = in_arena->pos;
    }

    void TempArena::end()
    {
        APORIA_VALIDATE_OR_RETURN(arena, "TempArena has an invalid arena! Can't rollback!");

        arena->pos = pos;
    }
}
