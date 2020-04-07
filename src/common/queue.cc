#pragma once

#include <cstdint>

// For the given type defines:
//    kMax<type> - The upper bound count for the given type.
//    k<type> - The storage for the type.
//    kRead<type> - Unsigned number of writes performed.
//    kWrite<type> - Unsigned number of reads performed.
// Methods:
//    Pop<type>() - return the next queued element
//    Push<type>(value) - append value to the queue
#define DECLARE_QUEUE(type, max_count)                    \
                                                          \
  static_assert((max_count & (max_count - 1)) == 0,       \
                "max_count must be a power of 2");        \
  constexpr uint64_t kMax##type = max_count;              \
                                                          \
  static type k##type[max_count];                         \
                                                          \
  static uint64_t kRead##type;                            \
  static uint64_t kWrite##type;                           \
                                                          \
  type Pop##type()                                        \
  {                                                       \
    if (kWrite##type - kRead##type == 0) return type{};   \
    type ret = k##type[kRead##type % kMax##type];         \
    kRead##type += 1;                                     \
    return ret;                                           \
  }                                                       \
                                                          \
  void Push##type(type val)                               \
  {                                                       \
    if (kWrite##type - kRead##type == kMax##type) return; \
    k##type[kWrite##type % kMax##type] = val;             \
    kWrite##type += 1;                                    \
  }                                                       \
                                                          \
  uint64_t Count##type()                                  \
  {                                                       \
    return kWrite##type - kRead##type;                    \
  }
