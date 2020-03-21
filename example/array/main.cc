#include <cstdio>
#include <cassert>

#include "common\array.cc"

struct Foo
{
  int x;
};

DECLARE_ND_ARRAY(Foo, 2, 10);

int
main()
{
  UseFoo(0)->x = 1;
  UseFoo(0)->x = 2;
  UseFoo(0)->x = 3;
  UseFoo(0)->x = 4;
  UseFoo(0)->x = 5;
  UseFoo(0)->x = 6;
  UseFoo(0)->x = 7;

  UseFoo(1)->x = 11;
  UseFoo(1)->x = 12;
  UseFoo(1)->x = 13;
  UseFoo(1)->x = 14;
  UseFoo(1)->x = 15;
  UseFoo(1)->x = 16;
  UseFoo(1)->x = 17;

  for (int i = 0; i < kUsedFoo[0]; ++i) {
    printf("%i\n", kFoo[0][i].x);
  }

  for (int i = 0; i < kUsedFoo[1]; ++i) {
    printf("%i\n", kFoo[1][i].x);
  }

  return 0;
}
