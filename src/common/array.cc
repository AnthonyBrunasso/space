#pragma once

#include <cstdint>

// For the given type defines:
//    kMax<type> - The upper bound count for the given type.
//    k<type> - The storage for the type.
//    kUsed<type> - The in-use count of the given type.
// Methods:
//    Use<type>() - Function to request use of a instance of type.
//    Release<type>() - Function to return an instance of type.
#define DECLARE_ARRAY(type, max_count)             \
  constexpr uint64_t kMax##type = max_count;       \
                                                   \
  static type k##type[max_count];                  \
  static type kZero##type;                         \
                                                   \
  static uint64_t kUsed##type;                     \
                                                   \
  type* Use##type()                                \
  {                                                \
    if (kUsed##type >= kMax##type) return nullptr; \
    type* t = &k##type[kUsed##type];               \
    kUsed##type += 1;                              \
    return t;                                      \
  }                                                \
                                                   \
