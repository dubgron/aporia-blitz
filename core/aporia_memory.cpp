#include "aporia_memory.hpp"

#include "debug/asserts.hpp"

#define PTR_TO_INT(p) reinterpret_cast<u64>(p)
#define INT_TO_PTR(i) reinterpret_cast<void*>(i)

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

        APORIA_CHECK( PTR_TO_INT(memory) % align == 0 );
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
        APORIA_VALIDATE_OR_RETURN_VALUE(pos + size <= max, nullptr,
            "Can't allocate {} B! Pos: {} B, Max: {} B, Left: {} B", size, pos, max, max - pos);

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

    void TempArena::rollback()
    {
        APORIA_VALIDATE_OR_RETURN(arena, "TempArena has an invalid arena! Can't rollback!");

        arena->pos = pos;
    }
}
