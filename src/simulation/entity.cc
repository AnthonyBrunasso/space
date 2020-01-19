
#include "common/common.cc"
#include "math/math.cc"

// Data used by game simulation
#define DECLARE_GAME_TYPE(type, count) DECLARE_ARRAY(type, count)
#define DECLARE_GAME_QUEUE(type, count) DECLARE_QUEUE(type, count)

struct Transform {
  math::Vec3f position;
  math::Vec3f scale = math::Vec3f(1.f, 1.f, 1.f);
  math::Quatf orientation;
};

enum CrewAttrib {
  CREWA_STR,
  CREWA_INT,
  CREWA_DEX,
  CREWA_CON,
  CREWA_MAX,
};

const char* const crew_aname[] = {
    "strength",
    "intelligence",
    "dexterity",
    "constitution",
};
#define CREW_ABEST \
  {                \
      18,          \
      18,          \
      18,          \
      18,          \
  };
#define CREW_AWORST \
  {                 \
      3,            \
      3,            \
      3,            \
      3,            \
  };

float
dsq(math::Vec3f dst, math::Vec3f src)
{
  math::Vec3f delta = dst - src;
  return delta.x * delta.x + delta.y * delta.y;
}

float
transform_dsq(Transform* dst, Transform* src)
{
  return dsq(dst->position, src->position);
}

struct Asteroid {
  Transform transform;
  uint64_t mineral_source;
  uint64_t flags = 0;
};
DECLARE_GAME_TYPE(Asteroid, 8);

constexpr uint64_t kPodMaxMineral = 100;
struct Pod {
  Transform transform;
  uint64_t think_flags = 0;
  math::Vec2f goal;
  math::Vec2f last_heading;
  uint64_t mineral;
};
DECLARE_GAME_TYPE(Pod, 8);

struct Command {
  enum Type {
    kNone = 0,
    kMine = 1,
    kMove = 2,
  };
  Type type;
  math::Vec2f destination;
};

DECLARE_GAME_QUEUE(Command, 16);
struct Unit {
  Transform transform;
  Command command;
  uint64_t think_flags = 0;
  int kind = 0;
  uint8_t acurrent[CREWA_MAX];
  uint8_t aknown_min[CREWA_MAX] = CREW_AWORST;
  uint8_t aknown_max[CREWA_MAX] = CREW_ABEST;
};

DECLARE_GAME_TYPE(Unit, 8);

struct Ship {
  uint64_t think_flags = 0;
  uint64_t crew_think_flags = 0;
  uint64_t mineral = 0;
  uint64_t danger = 0;
  float sys_power;
  float sys_engine;
  float sys_mine;
  float used_power;
  float power_delta;
};
DECLARE_GAME_TYPE(Ship, 1);

struct Missile {
  Transform transform;
  uint64_t flags;
};
DECLARE_GAME_TYPE(Missile, 8);

struct GameStatus {
  bool over;
};
DECLARE_GAME_TYPE(GameStatus, 1);
