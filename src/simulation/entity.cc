
#pragma once

#include "common/common.cc"
#include "math/math.cc"
#include "platform/platform.cc"

#include "entity_registry.cc"

// Data used by game simulation
#define DECLARE_GAME_TYPE(type, max_count)                               \
  DECLARE_ARRAY(type, max_count)                                         \
  static EntityRegistry kInit##type(k##type, &kZero##type, &kUsed##type, \
                                    max_count, sizeof(type));

#define DECLARE_GAME_QUEUE(type, count) DECLARE_QUEUE(type, count)

struct Transform {
  v3f position;
  v3f scale = v3f(1.f, 1.f, 1.f);
  math::Quatf orientation;
};

enum FtlFlag {
  kFtlTangible = 0,
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
dsq(v3f dst, v3f src)
{
  v3f delta = dst - src;
  return delta.x * delta.x + delta.y * delta.y;
}

float
transform_dsq(Transform* dst, Transform* src)
{
  return dsq(dst->position, src->position);
}

struct Command {
  enum Type {
    kNone = 0,
    kMine = 1,
    kMove = 2,
    kVacuum = 3,
  };
  Type type;
  v2f destination;
};

DECLARE_GAME_QUEUE(Command, 16);

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
  v2f goal;
  v2f last_heading;
  uint64_t mineral;
};
DECLARE_GAME_TYPE(Pod, 8);

struct Unit {
  Transform transform;
  v3f vacuum;
  Command command;
  uint64_t think_flags = 0;
  int kind = 0;
  uint8_t acurrent[CREWA_MAX];
  uint8_t aknown_min[CREWA_MAX] = CREW_AWORST;
  uint8_t aknown_max[CREWA_MAX] = CREW_ABEST;
};

DECLARE_GAME_TYPE(Unit, 8);

struct FtlState {
  // Number of frames the ftl is active: (frame - ftl_frame)
  uint64_t frame;
  // Stateful sideffect Flags
  uint64_t state_flags = FLAG(kFtlTangible);
};
struct Ship {
  uint64_t think_flags = 0;
  uint64_t crew_think_flags = 0;
  uint64_t mineral = 0;
  uint64_t danger = 0;
  float sys_power;
  float sys_engine;
  float sys_mine;
  float sys_turret;
  float used_power;
  float power_delta;
  uint64_t level;
  uint64_t frame;
  FtlState ftl;
  bool running;
};
DECLARE_GAME_TYPE(Ship, 1);

struct Missile {
  Transform transform;
  uint64_t flags;
  v2i tile_hit;
};
DECLARE_GAME_TYPE(Missile, 8);

