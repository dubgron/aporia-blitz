#pragma once

#include "aporia_types.hpp"

#define PTR_TO_INT(p) reinterpret_cast<uintptr_t>(p)
#define INT_TO_PTR(i) reinterpret_cast<void*>(i)

#define BYTES(n)      (n)
#define KILOBYTES(n)  (n << 10)
#define MEGABYTES(n)  (n << 20)
#define GIGABYTES(n)  (((u64)n) << 30)
#define TERABYTES(n)  (((u64)n) << 40)

namespace Aporia
{
    struct MemoryArena
    {
        void* memory = nullptr;
        u64 max = 0;
        u64 pos = 0;
        u64 align = 8;
    };

    MemoryArena arena_init(u64 size);
    void arena_deinit(MemoryArena* arena);

    void arena_clear(MemoryArena* arena);

    void* arena_push_uninitialized(MemoryArena* arena, u64 size);
    void* arena_push(MemoryArena* arena, u64 size);

    void arena_pop(MemoryArena* arena, u64 size);

    template<typename T>
    T* arena_push_uninitialized(MemoryArena* arena, u64 count = 1)
    {
        u64 size = count * sizeof(T);
        return (T*)arena_push_uninitialized(arena, size);
    }

    template<typename T>
    T* arena_push(MemoryArena* arena, u64 count = 1)
    {
        u64 size = count * sizeof(T);
        return (T*)arena_push(arena, size);
    }

    struct ScratchArena
    {
        MemoryArena* arena = nullptr;
        u64 pos = 0;
    };

    void temporary_memory_init(u64 size);
    void temporary_memory_deinit();

    ScratchArena scratch_begin(MemoryArena* conflict = nullptr);
    void scratch_end(ScratchArena scratch);
}
