
#include <cassert>
#include <cstdint>
#include <cstring>

#include <cstdio>

#include "common/array.cc"
#include "platform/macro.h"

struct Registry {
  void* ptr;
  void* zero_ptr;
  uint64_t* memb_count;
  uint32_t memb_max;
  uint32_t memb_size;
};

#define MAX_REGISTRY (PAGE / sizeof(Registry))
DECLARE_ARRAY(Registry, MAX_REGISTRY);

class EntityRegistry
{
 public:
  // Used in global static initialization
  EntityRegistry(void* buffer, void* zero, uint64_t* count_ptr, uint32_t max,
                 uint32_t size)
  {
    assert(kUsedRegistry < MAX_REGISTRY);
    kRegistry[kUsedRegistry] = {buffer, zero, count_ptr, max, size};
    kUsedRegistry += 1;
  }
};

uint64_t
RegistryCompact()
{
  uint64_t sum = 0;
  for (int i = 0; i < kUsedRegistry; ++i) {
    Registry* r = &kRegistry[i];
    int lower = 0;
    int upper = *r->memb_count - 1;
    // printf("Check %d [%lu entities]\n", i, *r->memb_count);
    const uint32_t memb_size = r->memb_size;
    const void* zero_ptr = r->zero_ptr;
    uint64_t count = 0;
    for (; lower <= upper; ++lower) {
      uint8_t* lower_ent = (uint8_t*)r->ptr + lower * memb_size;
      bool empty_lower = memcmp(lower_ent, zero_ptr, memb_size) == 0;
      if (empty_lower) {
        uint8_t* upper_ent = (uint8_t*)r->ptr + upper * memb_size;
        memcpy(lower_ent, upper_ent, memb_size);
        --upper;
        ++count;
      }
    }

    *r->memb_count -= count;
    sum += count;
  }

  return sum;
}
