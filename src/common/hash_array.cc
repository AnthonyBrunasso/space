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
    if (!id) return nullptr;                                                  \
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
  }                                                                           \
                                                                              \
  void Clear##type(u32 id)                                                    \
  {                                                                           \
    if (!id) return;                                                          \
    u32 hash = Hash##type(id);                                                \
    HashEntry* entry = &kHashEntry##type[hash];                               \
    if (entry->id != id) return;                                              \
    k##type[entry->array_idx] = {};                                           \
    *entry = {0, 0};                                                          \
    --kUsed##type;                                                            \
  }                                                                           \
                                                                              \
  void Swap##type(u32 id1, u32 id2)                                           \
  {                                                                           \
    if (id1 == id2) return;                                                   \
    if (!id1) return;                                                         \
    u32 hash1 = Hash##type(id1);                                              \
    HashEntry* entry1 = &kHashEntry##type[hash1];                             \
    if (!id2) return;                                                         \
    u32 hash2 = Hash##type(id2);                                              \
    HashEntry* entry2 = &kHashEntry##type[hash2];                             \
    type t = k##type[entry1->array_idx];                                      \
    k##type[entry1->array_idx] = k##type[entry2->array_idx];                  \
    k##type[entry2->array_idx] = t;                                           \
    u32 tarr = entry1->array_idx;                                             \
    entry1->array_idx = entry2->array_idx;                                    \
    entry2->array_idx = tarr;                                                 \
  }                                                                           \
                                                                              \
  void SwapAndClear##type(u32 id)                                             \
  {                                                                           \
    Swap##type(id, k##type[kUsed##type - 1].id);                              \
    Clear##type(id);                                                          \
  }                                                                           \
                                                                              \
  void Reset##type()                                                          \
  {                                                                           \
    for (u32 i = 0; i < kMax##type; ++i) {                                    \
      k##type[i] = {};                                                        \
    }                                                                         \
    for (u32 i = 0; i < kMaxHash##type; ++i) {                                \
      kHashEntry##type[i] = {};                                               \
    }                                                                         \
    kUsed##type = 0;                                                          \
    kAutoIncrementId##type = 1;                                               \
  }
