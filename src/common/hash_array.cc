#pragma once

#include <cassert>
#include <cstdint>

struct HashEntry {
  u32 id;
  u32 array_idx;
};

#define DECLARE_HASH_ARRAY(type, max_count)                                   \
  constexpr u32 kMax##type = max_count;                                       \
  constexpr u32 kMaxHash##type = (max_count * 2);                             \
  static_assert(POWEROF2(kMaxHash##type), "kMaxHash must be a power of 2");   \
                                                                              \
  static u32 kAutoIncrementId##type = 1;                                      \
  static u64 kUsed##type = 0;                                                 \
                                                                              \
  static type k##type[max_count];                                             \
  static HashEntry kHashEntry##type[kMaxHash##type];                          \
  static type kZero##type;                                                    \
                                                                              \
  b8 IsEmptyEntry##type(HashEntry entry)                                      \
  {                                                                           \
    return entry.id == kInvalidId || k##type[entry.array_idx].id != entry.id; \
  }                                                                           \
                                                                              \
  u32 Hash##type(u32 id)                                                      \
  {                                                                           \
    return MOD_BUCKET(id, kMaxHash##type);                                    \
  }                                                                           \
                                                                              \
  u32 GenerateFreeId##type()                                                  \
  {                                                                           \
    u32 id = kAutoIncrementId##type;                                          \
    HashEntry* hash_entry = &kHashEntry##type[Hash##type(id)];                \
    while (!IsEmptyEntry##type(*hash_entry)) {                                \
      id += 1;                                                                \
      hash_entry = &kHashEntry##type[Hash##type(id)];                         \
    }                                                                         \
    kAutoIncrementId##type = id;                                              \
    return id;                                                                \
  }                                                                           \
                                                                              \
  type* Use##type()                                                           \
  {                                                                           \
    assert(kUsed##type < max_count);                                          \
    if (kUsed##type >= max_count) return nullptr;                             \
    type* u = &k##type[kUsed##type++];                                        \
    *u = {};                                                                  \
    u->id = GenerateFreeId##type();                                           \
    u32 hash = Hash##type(u->id);                                             \
    HashEntry* hash_entry = &kHashEntry##type[hash];                          \
    hash_entry->id = u->id;                                                   \
    hash_entry->array_idx = kUsed##type - 1;                                  \
    ++kAutoIncrementId##type;                                                 \
    return u;                                                                 \
  }                                                                           \
                                                                              \
  type* Find##type(u32 id)                                                    \
  {                                                                           \
    u32 hash = Hash##type(id);                                                \
    HashEntry* entry = &kHashEntry##type[hash];                               \
    if (entry->id != id) return nullptr;                                      \
    return &k##type[entry->array_idx];                                        \
  }                                                                           \
                                                                              \
  void FreeHashEntry##type(u32 id)                                            \
  {                                                                           \
    u32 hash = Hash##type(id);                                                \
    kHashEntry##type[hash] = {0, 0};                                          \
  }
