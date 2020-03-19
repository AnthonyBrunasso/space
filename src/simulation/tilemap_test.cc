#include <cstdint>
#include <cstdio>

struct Tile {
  unsigned cx : 8;
  unsigned cy : 8;
  unsigned blocked : 1;
  unsigned nooxygen : 1;
  unsigned shroud : 1;
  unsigned explored : 1;
  unsigned exterior : 1;
};

Tile
TileAND(Tile lhs, Tile rhs)
{
  uint32_t res = *(uint32_t *)&lhs & *(uint32_t *)&rhs;
  return *(Tile *)&res;
}

Tile
TileOR(Tile lhs, Tile rhs)
{
  uint32_t res = *(uint32_t *)&lhs | *(uint32_t *)&rhs;
  return *(Tile *)&res;
}

void
ptile(Tile t)
{
  printf(
      "%ux %uy: [ blocked %d ] [ nooxygen %d ] [ shroud %d ] [ explored %d ]\n",
      t.cx, t.cy, t.blocked, t.nooxygen, t.shroud, t.explored);
}

int
main()
{
  Tile normal = {3, 3, 0, 0, 1, 1};
  Tile hull = {7, 7, 1, 0, 1, 1};
  Tile no_mutation = {0xff, 0xff, 1, 1, 1, 1};
  Tile no_shroud = {0xff, 0xff, 1, 1, 0, 1};
  Tile unexplored = {0xff, 0xff, 1, 1, 1, 0};
  Tile explored = {0x00, 0x00, 0, 0, 0, 1};

  puts("normal tile");
  ptile(normal);
  ptile(TileAND(normal, no_mutation));
  ptile(TileAND(normal, no_shroud));

  puts("hull tile");
  ptile(hull);
  ptile(TileAND(hull, no_mutation));
  ptile(TileAND(hull, no_shroud));

  puts("exploration");
  ptile(normal);
  ptile(TileAND(normal, unexplored));
  ptile(TileOR(normal, explored));

  return 0;
}
