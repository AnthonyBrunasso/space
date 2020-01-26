#include <iostream>

#include "math/vec.h"

struct Combined {
  uint64_t entity_id;
  v3f vec;
} __attribute__((packed));

int
main()
{
  std::cout << sizeof(math::Vec3i) << std::endl;
  std::cout << sizeof(uint64_t) << std::endl;
  std::cout << sizeof(Combined) << std::endl;

  v3f data;

  data.x = 0;
  data.y = 1;
  data.z = 2;

  int x = 0;
  int y = 1;
  int z = 2;

  return 0;
}
