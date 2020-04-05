#include <cstdio>

#include "common\array.cc"
#include "common\hash_array.cc"
#include "math\vec.h"

struct UnitH {
  HASH_STRUCT()

  v3f pos;
  v3f vel;
  v3f bounds;
};

DECLARE_HASH_ARRAY(UnitH, 1000);

struct UnitA {
  uint32_t id;
  v3f pos;
  v3f vel;
  v3f bounds;
};

DECLARE_ID_ARRAY(UnitA, 1000);

void
DebugPrint()
{
  printf("Unit - ");

  for (int i = 0; i < kMaxUnitH; ++i) {
    printf("%i/%llu/%llu ", i, kUnitH[i].id, kUnitH[i].hash_idx);
  }

  printf("\nHash - ");

  for (int i = 0; i < kMaxHashUnitH; ++i) {
    printf("%i/%llu/%llu ", i, kHashEntryUnitH[i].id, kHashEntryUnitH[i].array_idx);
  }

  printf("\n\n");
}
  
int
main()
{
  UseUnitH();
  UseUnitH(); 

  DebugPrint();

  return 0;
}
