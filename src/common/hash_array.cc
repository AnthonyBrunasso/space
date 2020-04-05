#pragma once

#include <cassert>
#include <cstdint>

#define HASH_STRUCT()  \
  uint64_t id;         \
  uint64_t hash_idx;   \

struct HashEntry {
  uint64_t id;
  uint64_t array_idx;
};

#define DECLARE_HASH_ARRAY(type, max_count)                         \
  constexpr uint64_t kMax##type = max_count;                        \
  constexpr uint64_t kMaxHash##type = (uint64_t)(1.3f * max_count); \
                                                                    \
  static uint64_t kAutoIncrementId##type = 1;                       \
  static uint64_t kUsed##type = 0;                                  \
  static uint64_t kInvalid##type = 0;                               \
                                                                    \
  static type k##type[max_count];                                   \
  static HashEntry kHashEntry##type[kMaxHash##type];                \
  static type kZero##type;                                          \
                                                                    \
  bool                                                              \
  IsEmptyEntry##type(HashEntry entry)                               \
  {                                                                 \
    return entry.id ==  kInvalid##type ||                           \
           k##type[entry.array_idx].id != entry.id;                 \
  }                                                                 \
                                                                    \
  HashEntry*                                                        \
  FindEmptyHashEntry##type(uint64_t hash, uint64_t* hash_idx)       \
  {                                                                 \
    *hash_idx = hash;                                               \
    HashEntry* hash_entry = &kHashEntry##type[*hash_idx];           \
    while (!IsEmptyEntry##type(*hash_entry)) {                      \
      printf("collides\n");                                         \
      *hash_idx += 1;                                               \
      *hash_idx = *hash_idx % kMaxHash##type;                       \
      hash_entry = &kHashEntry##type[*hash_idx];                    \
    }                                                               \
    return hash_entry;                                              \
  }                                                                 \
                                                                    \
  uint64_t                                                          \
  Hash##type(uint64_t id)                                           \
  {                                                                 \
    return (id * 2654435761 % kMaxHash##type);                      \
  }                                                                 \
                                                                    \
  type*                                                             \
  Use##type()                                                       \
  {                                                                 \
    if (kUsed##type >= kMax##type) return nullptr;                  \
    type* u = &k##type[kUsed##type++];                              \
    u->id = kAutoIncrementId##type++;                               \
    uint64_t hash = Hash##type(u->id);                              \
    HashEntry* hash_entry =                                         \
        FindEmptyHashEntry##type(hash, &u->hash_idx);               \
    hash_entry->id = u->id;                                         \
    hash_entry->array_idx = kUsed##type - 1;                        \
    return u;                                                       \
  }                                                                 \
                                                                    \
  type*                                                             \
  Find##type(uint64_t id)                                           \
  {                                                                 \
    uint64_t hash = Hash##type(id);                                 \
    type* u = &k##type[hash];                                       \
    uint64_t n = 1;                                                 \
    while (u->id != id) {                                           \
      if (n >= kMaxHash##type) return nullptr;                      \
      ++hash;                                                       \
      hash = hash % kMaxHash##type;                                 \
      u = &k##type[hash];                                           \
      ++n;                                                          \
    }                                                               \
    return u;                                                       \
  }                                                                 \
