
#include <cassert>
#include <cstdint>

#include "platform/macro.h"

struct GameArray {
  void* ptr;
  uint32_t memb_count;
  uint32_t memb_size;
};

#define MAX_GAMEARRAY (PAGE / sizeof(GameArray))
static GameArray kRegistry[MAX_GAMEARRAY] ALIGNAS(PAGE);
static uint64_t kUsedRegistry;

class EntityRegistry
{
 public:
  // Used in global static initialization
  EntityRegistry(void* buffer, uint32_t count, uint32_t size)
  {
    assert(kUsedRegistry < MAX_GAMEARRAY);
    kRegistry[kUsedRegistry] =
        (GameArray){.ptr = buffer, .memb_count = count, .memb_size = size};
    kUsedRegistry += 1;
  }
};

