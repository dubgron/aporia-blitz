#pragma once

#include "aporia_memory.hpp"
#include "aporia_string.hpp"

constexpr i64 FIRST_VALID_HASH = 0;
constexpr i64 NEVER_OCCUPIED_HASH = -1;
constexpr i64 REMOVED_HASH = -2;

constexpr i64 MAX_LOAD_FACTOR_PERCENT = 70;

// @NOTE(dubgron): Remember that this hash table does not automatically resize!
template<typename T>
struct HashTable
{
    struct Bucket
    {
        i64 hash = NEVER_OCCUPIED_HASH;
        String key;
        T value;
    };

    Bucket* buckets = nullptr;
    i64 bucket_count = 0;

    i64 valid_buckets = 0;
    i64 occupied_buckets = 0;
};

static u64 next_power_of_two(u64 n)
{
    APORIA_ASSERT(n != 0);

    u64 result = 1;
    while (n > result)
    {
        result += result;
    }
    return result;
}

template<typename T>
bool hash_table_is_created(HashTable<T>* hash_table)
{
    return hash_table->buckets && hash_table->bucket_count > 0;
}

template<typename T>
HashTable<T> hash_table_create(MemoryArena* arena, i64 bucket_count)
{
    // @NOTE(dubgron): The number of buckets in the hash table should be a power of 2.
    // For explanation, see 'hash_table_wrap_around' function.
    if ((bucket_count & (bucket_count - 1)) != 0)
    {
        bucket_count = next_power_of_two(bucket_count);
    }

    using HashTableBucket = typename HashTable<T>::Bucket;

    HashTable<T> result;
    result.buckets = arena_push<HashTableBucket>(arena, bucket_count);
    result.bucket_count = bucket_count;

    for (u64 idx = 0; idx < bucket_count; ++idx)
    {
        result.buckets[idx].hash = NEVER_OCCUPIED_HASH;
    }

    return result;
}

template<typename T>
void hash_table_destroy(HashTable<T>* hash_table)
{
    for (u64 idx = 0; idx < hash_table->bucket_count; ++idx)
    {
        using HashTableBucket = typename HashTable<T>::Bucket;
        hash_table->buckets[idx] = HashTableBucket{};
    }

    hash_table->valid_buckets = 0;
    hash_table->occupied_buckets = 0;
}

template<typename T>
void hash_table_expand(MemoryArena* arena, HashTable<T>* hash_table)
{
    u64 required_count;
    if ((hash_table->valid_buckets * 2 + 1) * 100 < MAX_LOAD_FACTOR_PERCENT * hash_table->bucket_count)
    {
        required_count = hash_table->bucket_count;
    }
    else
    {
        required_count = hash_table->bucket_count * 2;
    }

    HashTable<T> result = hash_table_create<T>(arena, required_count);

    for (u64 idx = 0; idx < hash_table->bucket_count; ++idx)
    {
        using HashTableBucket = typename HashTable<T>::Bucket;
        HashTableBucket* bucket = &hash_table->buckets[idx];

        if (bucket->hash >= FIRST_VALID_HASH)
        {
            hash_table_insert(&result, bucket->key, bucket->value);
        }
    }
}

template<typename T>
static inline i64 hash_table_wrap_around(HashTable<T>* hash_table, u64 value)
{
    // @NOTE(dubgron): The number of buckets in the hash table is a power
    // of 2, so we can easily wrap the value between 0 and bucket_count by
    // chopping off the top bits.
    return value & (hash_table->bucket_count - 1);
}

template<typename T>
T* hash_table_insert(HashTable<T>* hash_table, String key, T value)
{
    // @NOTE(dubgron): Without dividing, we want to test:
    //      occupied_buckets / bucket_count >= max_load_factor_percent / 100
    // Therefore, we say:
    //      occupied_buckets * 100 >= max_load_factor_percent * bucket_count
    if ((hash_table->occupied_buckets + 1) * 100 >= MAX_LOAD_FACTOR_PERCENT * hash_table->bucket_count)
    {
        APORIA_LOG(Warning, "The hash table is full! Failed to insert at key '%'!", key);
        return nullptr;
    }

    u32 hash = get_hash(key);
    i64 index = hash_table_wrap_around(hash_table, hash);

    u32 probe_increment = 1;

    // Probe forward, until you find an empty bucket.
    while (hash_table->buckets[index].hash != NEVER_OCCUPIED_HASH)
    {
        index = hash_table_wrap_around(hash_table, index + probe_increment);
        probe_increment += 1;
    }

    hash_table->buckets[index].hash = hash;
    hash_table->buckets[index].key = key;
    hash_table->buckets[index].value = value;

    hash_table->valid_buckets += 1;
    hash_table->occupied_buckets += 1;

    return &hash_table->buckets[index].value;
}

template<typename T>
T hash_table_remove(HashTable<T>* hash_table, String key)
{
    u32 hash = get_hash(key);
    i64 index = hash_table_wrap_around(hash_table, hash);

    u32 probe_increment = 1;

    // Probe forward, until you find the correct bucket (or an empty one).
    while (hash_table->buckets[index].hash != hash)
    {
        if (hash_table->buckets[index].hash == NEVER_OCCUPIED_HASH)
        {
            APORIA_LOG(Warning, "Tried to remove a non-existant key '%' from the hash table!", key);
            return T{};
        }

        index = hash_table_wrap_around(hash_table, index + probe_increment);
        probe_increment += 1;
    }

    hash_table->buckets[index].hash = REMOVED_HASH;
    hash_table->valid_buckets -= 1;

    return hash_table->buckets[index].value;
}

template<typename T>
T* hash_table_find(HashTable<T>* hash_table, String key)
{
    u32 hash = get_hash(key);
    i64 index = hash_table_wrap_around(hash_table, hash);

    u32 probe_increment = 1;

    // Probe forward, until you find the correct bucket (or an empty one).
    while (hash_table->buckets[index].hash != hash || hash_table->buckets[index].key != key)
    {
        if (hash_table->buckets[index].hash == NEVER_OCCUPIED_HASH)
        {
            return nullptr;
        }

        index = hash_table_wrap_around(hash_table, index + probe_increment);
        probe_increment += 1;
    }

    return &hash_table->buckets[index].value;
}
