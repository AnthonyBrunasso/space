#pragma once

#include <cassert>
#include <cstdint>

#define HASH_STRUCT()  \
  uint32_t id;         \

struct HashEntry {
  uint32_t id;
  uint32_t array_idx;
};

#define DECLARE_HASH_ARRAY(type, max_count)                         \
  constexpr uint32_t kMax##type = max_count;                        \
  constexpr uint32_t kMaxHash##type = (uint32_t)(1.3f * max_count); \
                                                                    \
  static uint32_t kAutoIncrementId##type = 1;                       \
  static uint64_t kUsed##type = 0;                                  \
  static uint32_t kInvalid##type = 0;                               \
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
  uint32_t                                                          \
  Hash##type(uint32_t id)                                           \
  {                                                                 \
    return (id * 2654435761 % kMaxHash##type);                      \
  }                                                                 \
                                                                    \
  uint32_t                                                          \
  GenerateFreeId##type()                                            \
  {                                                                 \
    uint32_t id = kAutoIncrementId##type;                           \
    HashEntry* hash_entry = &kHashEntry##type[Hash##type(id)];      \
    while (!IsEmptyEntry##type(*hash_entry)) {                      \
      printf("collides\n");                                         \
      id += 1;                                                      \
      hash_entry = &kHashEntry##type[Hash##type(id)];               \
    }                                                               \
    kAutoIncrementId##type = id;                                    \
    return id;                                                      \
  }                                                                 \
                                                                    \
  type*                                                             \
  Use##type()                                                       \
  {                                                                 \
    assert(kUsed##type < kMax##type);                               \
    if (kUsed##type >= kMax##type) return nullptr;                  \
    type* u = &k##type[kUsed##type++];                              \
    *u = {};                                                        \
    u->id = GenerateFreeId##type();                                 \
    uint32_t hash = Hash##type(u->id);                              \
    HashEntry* hash_entry = &kHashEntry##type[hash];                \
    hash_entry->id = u->id;                                         \
    hash_entry->array_idx = kUsed##type - 1;                        \
    ++kAutoIncrementId##type;                                       \
    return u;                                                       \
  }                                                                 \
                                                                    \
  type*                                                             \
  Find##type(uint32_t id)                                           \
  {                                                                 \
    uint32_t hash = Hash##type(id);                                 \
    type* u = &k##type[hash];                                       \
    uint32_t n = 1;                                                 \
    while (u->id != id) {                                           \
      if (n >= kMaxHash##type) return nullptr;                      \
      ++hash;                                                       \
      hash = hash % kMaxHash##type;                                 \
      u = &k##type[hash];                                           \
      ++n;                                                          \
    }                                                               \
    return u;                                                       \
  }                                                                 \
                                                                    \
  void                                                              \
  FreeHashEntry##type(uint32_t id)                                  \
  {                                                                 \
    uint32_t hash = Hash##type(id);                                 \
    HashEntry* hash_entry = &kHashEntry##type[hash];                \
    uint32_t n = 1;                                                 \
    while (hash_entry->id != id) {                                  \
      if (n >= kMaxHash##type) return;                              \
      ++hash;                                                       \
      hash = hash % kMaxHash##type;                                 \
      hash_entry = &kHashEntry##type[hash];                         \
      ++n;                                                          \
    }                                                               \
    *hash_entry = {0, 0};                                           \
  }

