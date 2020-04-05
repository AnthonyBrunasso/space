#include <cstdio>

#include "common\array.cc"
#include "common\hash_array.cc"
#include "math\vec.h"

#define N 1000

struct UnitH {
  HASH_STRUCT()

  v3f pos;
  v3f vel;
  v3f bounds;
};

DECLARE_HASH_ARRAY(UnitH, N);

struct UnitA {
  uint32_t id;
  v3f pos;
  v3f vel;
  v3f bounds;
};

DECLARE_ID_ARRAY(UnitA, N);

void
DebugPrintH()
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

void
DebugPrintA()
{
  printf("Unit - ");
  for (int i = 0; i < kMaxUnitA; ++i) {
    printf("%i/%u ", i, kUnitA[i].id);
  }

  printf("\n\n");
}

void
DebugPrintUnitH(UnitH* uh) {
  printf("Unit - %llu/%llu\n", uh->id, uh->hash_idx);
}

void
DebugPrintUnitA(UnitA* ua) {
  printf("Unit - %u\n", ua->id);
}
  
int
main()
{
  for (int i = 0; i < N; ++i) {
    UseUnitH();
    UseIdUnitA();
  }
  DebugPrintA();
  //DebugPrintUnitH(FindUnitH(445));
  //DebugPrintUnitA(FindUnitA(445));

  return 0;
}
