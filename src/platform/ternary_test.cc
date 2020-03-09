#include <cstdio>

#include "platform/macro.h"

int main()
{
  int x = 13;
  int tx = TERNARY(x == 13, 0, x);
  printf("Truth %d ? %d : %d; Value %d\n", x==13, 0, x, tx);

  x = 11;
  tx = TERNARY(x == 13, 0, x);
  printf("Truth %d ? %d : %d; Value %d\n", x == 13, 0, x, tx);

  float y = 13.0;
  float ty = TERNARY(y == 13.0f, 0.0f, y);
  printf("Truth %d ? %f : %f; Value %f\n", (y == 13.0f), 0.0, y, ty);

  y = 11.0f;
  ty = TERNARY(y == 13.0f, 0.0f, y);
  printf("Truth %d ? %f : %f; Value %f\n", (y == 13.0f), 0.0, y, ty);

  return 0;
}

