#include <cstdint>
#include <cstdio>

struct Tile {
  union {
    char position[6];
    struct {
      uint16_t cx;
      uint16_t cy;
      // Reference to the ship
      uint8_t ship_index;
      // for later
      uint8_t reserved;
    };
  };
  // Bitfields
  union {
    uint16_t flags;
    struct {
      // number of bits used in cx and cy
      uint16_t bitrange_xy : 4;
      // a wall, no movement
      uint16_t blocked : 1;
      // tile does not support human life
      uint16_t nooxygen : 1;
      // tile contents are 'unknown' unless marked visible
      uint16_t shroud : 1;
      // tile is visible to all players
      uint16_t visible : 1;
      // tile is exterior to the ship walls ('blocked' flag)
      uint16_t exterior : 1;
      // tile has ever been visible to a player
      uint16_t explored : 1;
    };
  };
};

void
ptile(Tile t)
{
  printf(
      "%ux %uy: "
      "[ blocked %d ] "
      "[ nooxygen %d ] "
      "[ shroud %d ] "
      "[ visible %d ] "
      "[ exterior %d ] "
      "[ explored %d ] "
      "[ flags %04x ] "
      "\n",
      t.cx, t.cy, t.blocked, t.nooxygen, t.shroud, t.visible, t.exterior, t.explored, t.flags);
}

int
main()
{
  Tile normal = {.cx = 7, .cy = 7, .ship_index = 0, .bitrange_xy = 4};

  puts("normal tile");
  ptile(normal);

  normal.exterior = 1;
  ptile(normal);

  normal.blocked = 1;
  ptile(normal);

  normal.explored = 1;
  normal.shroud = 1;
  ptile(normal);

  return 0;
}
