#pragma once

#include "aporia_types.hpp"

#define PTR_TO_INT(p) reinterpret_cast<uintptr_t>(p)
#define INT_TO_PTR(i) reinterpret_cast<void*>(i)

#define BYTES(n)      (n)
#define KILOBYTES(n)  (n << 10)
#define MEGABYTES(n)  (n << 20)
#define GIGABYTES(n)  (((u64)n) << 30)
#define TERABYTES(n)  (((u64)n) << 40)

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

//template<typename T>
//T* arena_push(MemoryArena* arena, u64 count = 1)
//{
//    u64 size = count * sizeof(T);
//    return (T*)arena_push(arena, size);
//}

// @NOTE(dubgron): This version is suboptimal if T is a type which is
// zero-initializable (i.e. it's a primitive type or is a struct without
// any non-zero default initializers for its members).
template<typename T>
T* arena_push(MemoryArena* arena, u64 count = 1)
{
    T* result = arena_push_uninitialized<T>(arena, count);
    for (u64 idx = 0; idx < count; ++idx)
    {
        result[idx] = T{};
    }
    return result;
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
