#pragma once

#include <cstdint>

// For the given type defines:
//    kMax<type> - The upper bound count for the given type.
//    k<type> - The storage for the type.
//    kUsed<type> - The in-use count of the given type.
// Methods:
//    Use<type>() - Function to request use of a instance of type.
//    Compress<type>(int idx) - Compresses the array starting at idx by moving
//    all elements that occur after idx down one element in the array.
#define DECLARE_ARRAY(type, max_count)             \
  constexpr u64 kMax##type = max_count;            \
                                                   \
  static type k##type[max_count];                  \
  static type kZero##type;                         \
                                                   \
  static u64 kUsed##type;                          \
                                                   \
  type* Use##type()                                \
  {                                                \
    assert(kUsed##type < kMax##type);              \
    if (kUsed##type >= kMax##type) return nullptr; \
    type* t = &k##type[kUsed##type];               \
    kUsed##type += 1;                              \
    *t = {};                                       \
    return t;                                      \
  }                                                \
                                                   \
  void Compress##type(int idx)                     \
  {                                                \
    if (idx >= kMax##type) return;                 \
    if (idx < 0) return;                           \
    for (int i = idx; i < kUsed##type; ++i) {      \
      k##type[i] = k##type[i + 1];                 \
    }                                              \
    --kUsed##type;                                 \
  }                                                \
                                                   \
  void Erase##type(int idx)                        \
  {                                                \
    if (idx >= kMax##type) return;                 \
    assert(idx < kUsed##type);                     \
    k##type[idx] = k##type[kUsed##type - 1];       \
    k##type[kUsed##type - 1] = {};                 \
    --kUsed##type;                                 \
  }

#define DECLARE_ID_ARRAY(type, max_count)                                 \
  DECLARE_ARRAY(type, max_count)                                          \
  static u32 kAutoIncrementId##type = 1;                                  \
                                                                          \
  type* UseId##type()                                                     \
  {                                                                       \
    type* t = Use##type();                                                \
    if (!t) return nullptr;                                               \
    t->id = kAutoIncrementId##type;                                       \
    kAutoIncrementId##type += (kAutoIncrementId##type == kInvalidId) + 1; \
    return t;                                                             \
  }                                                                       \
                                                                          \
  type* Find##type(u32 id)                                                \
  {                                                                       \
    for (s32 i = 0; i < kUsed##type; ++i) {                               \
      if (k##type[i].id == id) return &k##type[i];                        \
    }                                                                     \
    return nullptr;                                                       \
  }

#define DECLARE_2D_ARRAY(type, n, max_count)            \
  constexpr u64 kMax##type = max_count;                 \
  constexpr u64 kDim##type = n;                         \
  static type k##type[n][max_count];                    \
  static type kZero##type;                              \
                                                        \
  static u64 kUsed##type[n];                            \
                                                        \
  type* Use##type(u64 dim)                              \
  {                                                     \
    assert(dim < kDim##type);                           \
    assert(kUsed##type[dim] < kMax##type);              \
    if (kUsed##type[dim] >= kMax##type) return nullptr; \
    type* t = &k##type[dim][kUsed##type[dim]];          \
    kUsed##type[dim] += 1;                              \
    *t = {};                                            \
    return t;                                           \
  }                                                     \
                                                        \
  void Compress##type(u64 dim, int idx)                 \
  {                                                     \
    if (idx >= kMax##type) return;                      \
    if (idx < 0) return;                                \
    for (int i = idx; i < kUsed##type[dim]; ++i) {      \
      k##type[dim][i] = k##type[dim][i + 1];            \
    }                                                   \
    --kUsed##type[dim];                                 \
  }
