
#include <cassert>
#include <cstdint>
#include <cstring>

#include <cstdio>

#include "common/array.cc"
#include "common/hash_array.cc"
#include "platform/macro.h"

struct Registry {
  void* ptr;
  void* zero_ptr;
  uint64_t* memb_count;
  uint32_t memb_max;
  uint32_t memb_size;
  HashEntry* hash_entry;
  uint32_t (*hash_func)(uint32_t id);
};

#define MAX_REGISTRY (PAGE / sizeof(Registry))
DECLARE_ARRAY(Registry, MAX_REGISTRY);

class EntityRegistry
{
 public:
  // Used in global static initialization
  EntityRegistry(void* buffer, void* zero, uint64_t* count_ptr, uint32_t max,
                 uint32_t size, HashEntry* hash_entry,
                 uint32_t (*hash_func)(uint32_t))
  {
    assert(kUsedRegistry < MAX_REGISTRY);
    kRegistry[kUsedRegistry] =
        {buffer, zero, count_ptr, max, size, hash_entry, hash_func};
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
    uint64_t idx = 0;
    for (; lower <= upper; ++lower) {
      uint8_t* lower_ent = (uint8_t*)r->ptr + lower * memb_size;
      bool empty_lower = memcmp(lower_ent, zero_ptr, memb_size) == 0;
      if (empty_lower) {
        uint8_t* upper_ent = (uint8_t*)r->ptr + upper * memb_size;
        memcpy(lower_ent, upper_ent, memb_size);
        if (r->hash_entry) {
          // Expectation that id is first 32 bits in entities.
          uint32_t id = *((uint32_t*)(lower_ent));
          r->hash_entry[r->hash_func(id)].array_idx = idx;
        }
        memcpy(upper_ent, zero_ptr, memb_size);
        --upper;
        ++count;
      }
      ++idx;
    }

    *r->memb_count -= count;
    sum += count;
  }

  return sum;
}
