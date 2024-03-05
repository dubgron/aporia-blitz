#pragma once

#include "aporia_memory.hpp"
#include "aporia_string.hpp"

namespace Aporia
{
    struct HashTableKey
    {
        i32 distance_from_desired_bucket = -1;
        String key;
    };

    // @NOTE(dubgron): Remember that this hash table is not resizable!
    template<typename T>
    struct HashTable
    {
        HashTableKey* keys = nullptr;
        T* buckets = nullptr;

        u64 bucket_count = 0;
        u64 occupied_buckets = 0;
    };

    static u64 hash_to_index(u64 bucket_count, u32 hash)
    {
        // @NOTE(dubgron): The number of buckets in the hash table is a power
        // of 2, so we can easily fold the hash between 0 and bucket_count by
        // chopping off the top bits, which is less robust than using an integer
        // modulo operation, but it's way faster.
        return hash & (bucket_count - 1);
    }

    static u64 increment_index(u64 bucket_count, u64 index)
    {
        return index < bucket_count - 1 ? (index + 1) : 0;
    }

    static u64 decrement_index(u64 bucket_count, u64 index)
    {
        return index > 0 ? index - 1 : bucket_count - 1;
    }

    template<typename T>
    bool hash_table_is_created(HashTable<T>* hash_table)
    {
        return hash_table->keys && hash_table->buckets && hash_table->bucket_count > 0;
    }

    template<typename T>
    HashTable<T> hash_table_create(MemoryArena* arena, u64 bucket_count)
    {
        // @NOTE(dubgron): The number of buckets in the hash table should be a power of 2.
        // For explanation, see 'hash_to_index' function.
        APORIA_ASSERT((bucket_count & (bucket_count - 1)) == 0);

        HashTable<T> result;
        result.keys = arena_push<HashTableKey>(arena, bucket_count);
        result.buckets = arena_push<T>(arena, bucket_count);
        result.bucket_count = bucket_count;

        for (u64 idx = 0; idx < bucket_count; ++idx)
        {
            result.keys[idx].distance_from_desired_bucket = -1;
        }

        return result;
    }

    template<typename T>
    void hash_table_destroy(HashTable<T>* hash_table)
    {
        for (u64 idx = 0; idx < hash_table->bucket_count; ++idx)
        {
            hash_table->keys[idx] = HashTableKey{};
            hash_table->buckets[idx] = 0;
        }
    }

    template<typename T>
    T* hash_table_insert(HashTable<T>* hash_table, String key, T value)
    {
        if (hash_table->occupied_buckets >= hash_table->bucket_count)
        {
            APORIA_LOG(Error, "The hash table is full! Failed to insert at key '%'!", key);
            return nullptr;
        }

        u32 hashed_key = hash(key);
        u64 index = hash_to_index(hash_table->bucket_count, hashed_key);

        HashTableKey ht_key;
        ht_key.key = key;
        ht_key.distance_from_desired_bucket = 0;

        while (hash_table->keys[index].distance_from_desired_bucket != -1)
        {
            if (ht_key.distance_from_desired_bucket > hash_table->keys[index].distance_from_desired_bucket)
            {
                HashTableKey temp_key = hash_table->keys[index];
                hash_table->keys[index] = ht_key;
                ht_key = temp_key;

                T temp_value = hash_table->buckets[index];
                hash_table->buckets[index] = value;
                value = temp_value;
            }

            ht_key.distance_from_desired_bucket += 1;

            index = increment_index(hash_table->bucket_count, index);
        }

        hash_table->keys[index] = ht_key;
        hash_table->buckets[index] = value;

        hash_table->occupied_buckets += 1;

        return &hash_table->buckets[index];
    }

    template<typename T>
    T hash_table_remove(HashTable<T>* hash_table, String key)
    {
        u32 hashed_key = hash(key);
        u64 index = hash_to_index(hash_table->bucket_count, hashed_key);

        // Probe forward, until you find the correct bucket (or an empty one).
        while (hash_table->keys[index].key != key)
        {
            index = increment_index(hash_table->bucket_count, index);

            if (hash_table->keys[index].distance_from_desired_bucket == -1)
            {
                APORIA_LOG(Warning, "Tried to remove a non-existant key '%' from the hash table!", key);
                return T{};
            }
        }

        T result = hash_table->buckets[index];

        hash_table->keys[index] = HashTableKey{};
        hash_table->buckets[index] = T{};

        index = increment_index(hash_table->bucket_count, index);

        while (hash_table->keys[index].distance_from_desired_bucket > 0)
        {
            u64 prev_index = decrement_index(hash_table->bucket_count, index);

            hash_table->keys[prev_index] = hash_table->keys[index];
            hash_table->keys[prev_index].distance_from_desired_bucket -= 1;
            hash_table->keys[index] = HashTableKey{};

            hash_table->buckets[prev_index] = hash_table->buckets[index];
            hash_table->buckets[index] = T{};

            index = increment_index(hash_table->bucket_count, index);
        }

        return result;
    }

    template<typename T>
    T* hash_table_find(HashTable<T>* hash_table, String key)
    {
        u32 hashed_key = hash(key);
        u64 index = hash_to_index(hash_table->bucket_count, hashed_key);

        if (hash_table->keys[index].distance_from_desired_bucket == -1)
        {
            return nullptr;
        }

        u64 buckets_to_check = hash_table->bucket_count;

        // Probe forward, until you find the correct bucket (or an empty one).
        while (buckets_to_check > 0 && hash_table->keys[index].key != key)
        {
            index = increment_index(hash_table->bucket_count, index);
            buckets_to_check -= 1;

            if (hash_table->keys[index].distance_from_desired_bucket == -1)
            {
                return nullptr;
            }
        }

        if (buckets_to_check == 0)
        {
            return nullptr;
        }

        return &hash_table->buckets[index];
    }
}
