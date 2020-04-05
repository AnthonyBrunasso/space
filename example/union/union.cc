#include <cstdint>
#include <cstdio>

#define COMMON_MEMBER_DECL                                                     \
  int x = 0;                                                                   \
  int y = 0;                                                                   \
  int type

enum EntityEnum {
  kEeUnit = 0,
  kEeAlien = 1,
};

struct Unit {
  COMMON_MEMBER_DECL = kEeUnit;
  float uaction;
};

struct Alien {
  COMMON_MEMBER_DECL = kEeAlien;
  float ai_state;
};

union Entity {
  struct {
    COMMON_MEMBER_DECL = 500;
  };
  Unit unit;
  Alien alien;

  Entity() : type(500) {}
};

int main() {
  Entity e = {};
  printf("default {} %d %d %d\n", e.x, e.y, e.type);
  e.x = 1;
  e.y = 2;
  e.type = 1000;
  printf("assigned %d %d %d\n", e.x, e.y, e.type);
  Alien a = {};
  printf("Alien %d %d %d\n", a.x, a.y, a.type);
  Unit u = {};
  printf("Unit %d %d %d\n", u.x, u.y, u.type);

  return 0;
}

