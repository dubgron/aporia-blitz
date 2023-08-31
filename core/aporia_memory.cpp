#include "aporia_memory.hpp"

#include "aporia_debug.hpp"

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
        APORIA_ASSERT_WITH_MESSAGE(size > 0 && size <= space_left,
            "Can't allocate % B! Pos: % B, Max: % B, Left: % B", size, pos, max, space_left);

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

    ScratchArena create_scratch_arena(MemoryArena* arena)
    {
        APORIA_ASSERT_WITH_MESSAGE(arena, "The arena is null! Can't create a scratch arena!");

        ScratchArena result;
        result.arena = arena;
        result.pos = arena->pos;
        return result;
    }

    void rollback_scratch_arena(ScratchArena& scratch)
    {
        APORIA_ASSERT_WITH_MESSAGE(scratch.arena, "Scratch arena has an invalid arena pointer! Can't rollback!");
        scratch.arena->pos = scratch.pos;
    }
}
