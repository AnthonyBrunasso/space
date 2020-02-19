#include <cstdint>
#include <cstdio>

struct Tile {
  unsigned cx : 8;
  unsigned cy : 8;
  unsigned blocked : 1;
  unsigned nooxygen : 1;
  unsigned shroud : 1;
};

Tile TileAND(Tile lhs, Tile rhs) {
  uint32_t res = *(uint32_t *)&lhs & *(uint32_t *)&rhs;
  return *(Tile *)&res;
}

void ptile(Tile t) {
  printf("%ux %uy: [ blocked %d ] [ nooxygen %d ] [ shroud %d ]\n", t.cx, t.cy,
         t.blocked, t.nooxygen, t.shroud);
}

int main() {
  Tile normal = {3, 3, 0, 0, 1};
  Tile hull = {7, 7, 1, 0, 1};
  Tile no_mutation = {0xff, 0xff, 1, 1, 1};
  Tile no_shroud = {0xff, 0xff, 1, 1, 0};

  puts("normal tile");
  ptile(normal);
  ptile(TileAND(normal, no_mutation));
  ptile(TileAND(normal, no_shroud));

  puts("hull tile");
  ptile(hull);
  ptile(TileAND(hull, no_mutation));
  ptile(TileAND(hull, no_shroud));

  return 0;
}
