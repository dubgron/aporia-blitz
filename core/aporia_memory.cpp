#include "aporia_memory.hpp"

#include "aporia_debug.hpp"
#include "aporia_utils.hpp"

static u64 next_aligned(u64 address, u64 align)
{
    APORIA_ASSERT((align & (align - 1)) == 0);
    u64 aligned_address = (address + align - 1) & ~(align - 1);
    return aligned_address;
}

MemoryArena arena_init(u64 size)
{
    MemoryArena result;
    result.memory = malloc(size);
    result.max = size;
    result.pos = 0;
    result.align = 8;
    APORIA_ASSERT(PTR_TO_INT(result.memory) % result.align == 0);

    return result;
}

void arena_deinit(MemoryArena* arena)
{
    free(arena->memory);
    arena->memory = nullptr;
    arena->max = 0;
    arena->pos = 0;
    arena->align = 8;
}

void arena_clear(MemoryArena* arena)
{
    arena->pos = 0;
}

void* arena_push_uninitialized(MemoryArena* arena, u64 size)
{
    u64 space_left = arena->max - arena->pos;
    APORIA_ASSERT_WITH_MESSAGE(size > 0 && size <= space_left,
        "Can't allocate % B! Pos: % B, Max: % B, Left: % B", size, arena->pos, arena->max, space_left);

    u64 result = PTR_TO_INT(arena->memory) + arena->pos;
    arena->pos = next_aligned(arena->pos + size, arena->align);

    return INT_TO_PTR(result);
}

void* arena_push(MemoryArena* arena, u64 size)
{
    void* result = arena_push_uninitialized(arena, size);
    memset(result, 0, size);
    return result;
}

void arena_pop(MemoryArena* arena, u64 size)
{
    APORIA_ASSERT_WITH_MESSAGE(size > 0 && size <= arena->pos,
        "Can't pop % B! Pos: % B", size, arena->pos);

    arena->pos = next_aligned(arena->pos - size, arena->align);
}

static MemoryArena temporary[2];

void temporary_memory_init(u64 size)
{
    for (u64 idx = 0; idx < ARRAY_COUNT(temporary); ++idx)
    {
        temporary[idx] = arena_init(size);
    }
}

void temporary_memory_deinit()
{
    for (u64 idx = 0; idx < ARRAY_COUNT(temporary); ++idx)
    {
        arena_deinit(&temporary[idx]);
    }
}

ScratchArena scratch_begin(MemoryArena* conflict /* = nullptr */)
{
    ScratchArena result;
    for (u64 idx = 0; idx < ARRAY_COUNT(temporary); ++idx)
    {
        MemoryArena* temp_arena = &temporary[idx];
        if (conflict != temp_arena)
        {
            result.arena = temp_arena;
            result.pos = temp_arena->pos;
            break;
        }
    }
    return result;
}

void scratch_end(ScratchArena scratch)
{
    APORIA_ASSERT_WITH_MESSAGE(scratch.arena, "Scratch arena has an invalid arena pointer!");

#if defined(APORIA_DEBUGTOOLS)
    APORIA_ASSERT(scratch.arena->pos >= scratch.pos);

    // @NOTE(dubgron): Using scratch arenas enables a peculiar use-after-free
    // bug, where you create a scratch arena on top of arena X, you push some
    // values to it (which are meant to be temporary), you push some values to
    // arena X (which are not meant to be temporary), and you free the scratch
    // arena at the end. In this case, the values pushed directly onto arena X
    // would be treated as freed, and could be potentially overwritten later.
    // These sorts of bugs are near impossible to catch (without using static
    // code analysis), but we can make them easier to detect. In debug builds,
    // we clear the memory used by a scratch arena with a value of 0xae and
    // thus making the program either crash immediately (making tracking the
    // source of the bug easier) or have a simple to spot pattern in memory.

    void* scratch_block_begin = INT_TO_PTR(PTR_TO_INT(scratch.arena->memory) + scratch.pos);
    u64 scratch_block_size = scratch.arena->pos - scratch.pos;
    memset(scratch_block_begin, 0xae, scratch_block_size);
#endif

    scratch.arena->pos = scratch.pos;
}
