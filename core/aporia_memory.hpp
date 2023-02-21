#pragma once

#include "aporia_types.hpp"

#define Bytes(n)      (n)
#define Kilobytes(n)  (n << 10)
#define Megabytes(n)  (n << 20)
#define Gigabytes(n)  (((u64)n) << 30)
#define Terabytes(n)  (((u64)n) << 40)

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

        template<typename T>
        T* push(u64 count)
        {
            const u64 size = count * sizeof(T);
            return reinterpret_cast<T*>( push(size) );
        }

        template<typename T>
        T* push_zero(u64 count)
        {
            const u64 size = count * sizeof(T);
            return reinterpret_cast<T*>( push_zero(size) );
        }
    };

    struct TempArena
    {
        MemoryArena* arena = nullptr;
        u64 pos = 0;

        void begin(MemoryArena* in_arena);
        void rollback();
    };
}
