
#include <cassert>
#include <cstdint>

#include "common/array.cc"
#include "platform/macro.h"

struct Registry {
  void* ptr;
  uint32_t memb_count;
  uint32_t memb_size;
};

#define MAX_REGISTRY (PAGE / sizeof(Registry))
DECLARE_ARRAY(Registry, MAX_REGISTRY);

class EntityRegistry
{
 public:
  // Used in global static initialization
  EntityRegistry(void* buffer, uint32_t count, uint32_t size)
  {
    assert(kUsedRegistry < MAX_REGISTRY);
    kRegistry[kUsedRegistry] =
        (Registry){.ptr = buffer, .memb_count = count, .memb_size = size};
    kUsedRegistry += 1;
  }
};

