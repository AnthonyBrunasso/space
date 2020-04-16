#include "common/common.cc"

#include <stdio.h>

struct Pane {
  const char* title;
};

DECLARE_HASH_MAP_STR(Pane, 8);

int
main()
{
  printf("%i\n", FindPane("test", 4) == nullptr);
  Pane* pane = UsePane("test", 4);
  pane->title = "test";
  printf("%s\n", FindPane("test", 4)->title);
  return 0;
}
