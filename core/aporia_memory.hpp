#pragma once

#include "aporia_types.hpp"

#define PTR_TO_INT(p) reinterpret_cast<u64>(p)
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

        void alloc(u64 size);
        void dealloc();

        void clear();

        void* push(u64 size);
        void* push_zero(u64 size);

        void pop(u64 size);

        template<typename T>
        T* push(u64 count = 1)
        {
            const u64 size = count * sizeof(T);
            return reinterpret_cast<T*>( push(size) );
        }

        template<typename T>
        T* push_zero(u64 count = 1)
        {
            const u64 size = count * sizeof(T);
            return reinterpret_cast<T*>( push_zero(size) );
        }

        template<typename T, u64 N>
        void make_from_array(T (&array)[N])
        {
            memory = array;
            max = N * sizeof(T);
        }
    };

    struct ScratchArena
    {
        MemoryArena* arena = nullptr;
        u64 pos = 0;
    };

    ScratchArena get_scratch_arena(MemoryArena* conflict = nullptr);
    void release_scratch_arena(ScratchArena& scratch);
}
