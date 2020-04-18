#pragma once

#include <cassert>
#include <cstdint>

constexpr uint32_t kMaxHashKeyLength = 32;

struct HashMapStrEntry {
  char str[kMaxHashKeyLength];
  uint32_t len = 0;
  uint32_t array_index;
};

bool
CompareHashMapEntry(const char* str, uint32_t len,
                    const HashMapStrEntry& entry)
{
  if (len != entry.len) return false;
  return strncmp(str, entry.str, len) == 0;
}

uint32_t
GetHash(const char* str, uint32_t len)
{
  // djb2_hash_more collides a lot for some reason - using this instead.
  // Call imui::DebugPane to see hash collisions on panes.
  uint32_t hash = 7;
  for (int i = 0; i < len; ++i) {
    hash = hash * 31 + str[i];
  }
  return hash;
}

#define DECLARE_HASH_MAP_STR(type, max_count)                       \
  constexpr uint32_t kMax##type = max_count;                        \
  constexpr uint32_t kMaxHash##type = (uint32_t)(1.3f * max_count); \
                                                                    \
  static uint64_t kUsed##type = 0;                                  \
  static uint32_t kInvalid##type = 0;                               \
                                                                    \
  static type k##type[max_count];                                   \
  static HashMapStrEntry kHashEntry##type[kMaxHash##type];          \
  static type kZero##type;                                          \
  static uint32_t kFindCalls##type = 0;                             \
  static uint32_t kFindCollisions##type = 0;                        \
                                                                    \
  HashMapStrEntry*                                                  \
  FindEmptyHashEntry##type(const char* key, uint32_t key_len)       \
  {                                                                 \
    uint32_t idx = GetHash(key, key_len) % kMaxHash##type;          \
    while (kHashEntry##type[idx].len) {                             \
      idx = (idx + 1) % kMaxHash##type;                             \
    }                                                               \
    return &kHashEntry##type[idx];                                  \
  }                                                                 \
                                                                    \
  type*                                                             \
  Use##type(const char* key, uint32_t key_len)                      \
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
  type*                                                             \
  Find##type(const char* key, uint32_t key_len)                     \
  {                                                                 \
    uint32_t idx = GetHash(key, key_len) % kMaxHash##type;          \
    HashMapStrEntry* hash_entry = &kHashEntry##type[idx];           \
    uint32_t n = 0;                                                 \
    ++kFindCalls##type;                                             \
    bool collision = false;                                         \
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
    return &k##type[hash_entry->array_index];                       \
  }                                                                 \
