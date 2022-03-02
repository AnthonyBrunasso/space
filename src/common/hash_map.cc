#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

constexpr u32 kMaxHashKeyLength = 128;

struct HashMapStrEntry {
  char str[kMaxHashKeyLength];
  u32 len = 0;
  u32 array_index = 0;
};

b8 CompareHashMapEntry(const char* str, u32 len, const HashMapStrEntry& entry) {
  if (len != entry.len) return false;
  return strncmp(str, entry.str, len) == 0;
}

u32 GetHash(const char* str, u32 len) {
  // djb2_hash_more collides a lot for some reason - using this instead.
  // Call imui::DebugPane to see hash collisions on panes.
  u32 hash = 7;
  for (s32 i = 0; i < len; ++i) {
    hash = hash * 31 + str[i];
  }
  return hash;
}

#define DECLARE_HASH_MAP_STR(type, max_count)                       \
  constexpr u32 kMax##type = max_count;                             \
  constexpr u32 kMaxHash##type = (u32)(1.3f * max_count);           \
                                                                    \
  static u64 kUsed##type = 0;                                       \
  static u32 kInvalid##type = 0;                                    \
                                                                    \
  static type k##type[max_count];                                   \
  static HashMapStrEntry kHashEntry##type[kMaxHash##type];          \
  static type kZero##type;                                          \
  static u32 kFindCalls##type = 0;                                  \
  static u32 kFindCollisions##type = 0;                             \
                                                                    \
  HashMapStrEntry*                                                  \
  FindEmptyHashEntry##type(const char* key, u32 key_len)            \
  {                                                                 \
    u32 idx = GetHash(key, key_len) % kMaxHash##type;               \
    while (kHashEntry##type[idx].len) {                             \
      idx = (idx + 1) % kMaxHash##type;                             \
    }                                                               \
    return &kHashEntry##type[idx];                                  \
  }                                                                 \
                                                                    \
  type*                                                             \
  Use##type(const char* key, u32 key_len)                           \
  {                                                                 \
    assert(key_len <= kMaxHashKeyLength);                           \
    if (kUsed##type >= kMax##type) return nullptr;                  \
    type* u = &k##type[kUsed##type++];                              \
    *u = {};                                                        \
    HashMapStrEntry* entry = FindEmptyHashEntry##type(key, key_len);\
    memcpy(entry->str, key, key_len);                               \
    entry->len = key_len;                                           \
    entry->array_index = kUsed##type - 1;                           \
    return u;                                                       \
  }                                                                 \
                                                                    \
  HashMapStrEntry*                                                  \
  FindHashEntry##type(const char* key, u32 key_len)                 \
  {                                                                 \
    u32 idx = GetHash(key, key_len) % kMaxHash##type;               \
    HashMapStrEntry* hash_entry = &kHashEntry##type[idx];           \
    u32 n = 0;                                                      \
    ++kFindCalls##type;                                             \
    b8 collision = false;                                           \
    while (!CompareHashMapEntry(key, key_len, *hash_entry) &&       \
           n < kMaxHash##type) {                                    \
      idx = (idx + 1) % kMaxHash##type;                             \
      hash_entry = &kHashEntry##type[idx];                          \
      ++n;                                                          \
      collision = true;                                             \
    }                                                               \
    if (collision) {                                                \
      ++kFindCollisions##type;                                      \
    }                                                               \
    if (n >= kMaxHash##type) return nullptr;                        \
    return hash_entry;                                              \
  }                                                                 \
                                                                    \
  type*                                                             \
  Find##type(const char* key, u32 key_len)                          \
  {                                                                 \
    HashMapStrEntry* hash_entry = FindHashEntry##type(key, key_len);\
    if (!hash_entry) return nullptr;                                \
    return &k##type[hash_entry->array_index];                       \
  }                                                                 \
                                                                    \
  type*                                                             \
  FindOrUse##type(const char* key, u32 key_len)                     \
  {                                                                 \
    HashMapStrEntry* hash_entry = FindHashEntry##type(key, key_len);\
    if (!hash_entry) return Use##type(key, key_len);                \
    return &k##type[hash_entry->array_index];                       \
  }                                                                 \
                                                                    \
  void                                                              \
  Erase##type(const char* key, u32 key_len)                         \
  {                                                                 \
    HashMapStrEntry* hash_entry = FindHashEntry##type(key, key_len);\
    if (!hash_entry) return;                                        \
    u32 arr_idx = hash_entry->array_index;                          \
    *hash_entry = {};                                               \
    for (s32 i = arr_idx; i < kUsed##type; ++i) {                   \
      k##type[i] = k##type[i + 1];                                  \
    }                                                               \
    k##type[kUsed##type - 1] = {};                                  \
    --kUsed##type;                                                  \
    for (s32 i = 0; i < kMaxHash##type; ++i) {                      \
      HashMapStrEntry* hash_entry = &kHashEntry##type[i];           \
      if (!hash_entry->len) continue;                               \
      if (hash_entry->array_index > arr_idx)                        \
        --hash_entry->array_index;                                  \
    }                                                               \
  }
