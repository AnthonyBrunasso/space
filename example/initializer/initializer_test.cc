#include "simulation/entity.cc"

#include <cstdio>
#include <cstring>

void
dumphex(uint8_t* ptr)
{
  for (int i = 0; i < sizeof(Notify); ++i) {
    uint8_t* byte = ptr + i;
    printf("%02X", *byte);
  }
  puts("");
}

int
main()
{
  static Notify zeron;
  Notify n;
  n.age = 31;
  n = {};
  printf("%d\n", memcmp(&n, &zeron, sizeof(Notify)));

  memset(&n, 0xff, sizeof(Notify));
  n = {};
  printf("%d\n", memcmp(&n, &zeron, sizeof(Notify)));
  dumphex((uint8_t*)&n);
  dumphex((uint8_t*)&zeron);

  return 0;
}
