
#pragma once

#include "common/common.cc"
#include "math/math.cc"
#include "platform/platform.cc"

#include "camera.cc"
#include "entity_registry.cc"

// Data used by game simulation
#define DECLARE_GAME_TYPE(type, max_count)                               \
  DECLARE_ARRAY(type, max_count)                                         \
  static EntityRegistry kInit##type(k##type, &kZero##type, &kUsed##type, \
                                    max_count, sizeof(type));

#define DECLARE_GAME_TYPE_WITH_ID(type, max_count)                       \
  DECLARE_ARRAY(type, max_count)                                         \
  static int kAutoIncrementId##type = 0;                                 \
                                                                         \
  type* UseId##type() {                                                  \
    type* t = Use##type();                                               \
    if (!t) return nullptr;                                              \
    t->id = kAutoIncrementId##type;                                      \
    ++kAutoIncrementId##type;                                            \
    return t;                                                            \
  }                                                                      \
                                                                         \
  type* Find##type(int id) {                                             \
    for (int i = 0; i < kUsed##type; ++i) {                              \
      if (k##type[i].id == id) return &k##type[i];                       \
    }                                                                    \
    return nullptr;                                                      \
  }                                                                      \
  static EntityRegistry kInit##type(k##type, &kZero##type, &kUsed##type, \
                                    max_count, sizeof(type));

#define DECLARE_GAME_QUEUE(type, count) DECLARE_QUEUE(type, count)

#define GAME_ITER(type, member)                                   \
  sizeof(type), (const uint8_t*)k##type + offsetof(type, member), \
      (const uint8_t*)&k##type[kUsed##type]

// Common Flags
enum UnitAction {
  kUaNone = 0,
  kUaMove,
  kUaOperate,
  kUaVacuum,
  kUaAttack,
};
enum UnitKind {
  kPlayerControlled,
  kPowerOperator,
  kMiner,
  kEngineer,
  kTurretOperator,
  kEnemy
};
enum ShipAiGoals {
  kShipAiSpawnPod,
  kShipAiPowerSurge,
  kShipAiGoals = 64,
};
enum UnitAiGoals {
  kUnitAiPower = 0,
  kUnitAiMine,
  kUnitAiThrust,
  kUnitAiTurret,
  kUnitAiSavePower,
  kUnitAiGoals = 64,
};
enum UnitBbEntry {
  kUnitTarget = 0,
  kUnitDestination = 1,
  kUnitBbEntryMax = 2,
};
// TODO (AN): Document/test priority behaviors
enum PodAiGoals {
  kPodAiLostPower,
  kPodAiUnmanned,
  kPodAiGather,
  kPodAiUnload,
  kPodAiReturn,
  kPodAiApproach,
  kPodAiGoals = 64,
};

// Common Structures
struct Transform {
  v3f position;
  v3f scale = v3f(1.f, 1.f, 1.f);
};

// TODO (AN): Move crew
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
#define CREW_A(val) \
  {                 \
      val,          \
      val,          \
      val,          \
      val,          \
  };
#define CREW_ABEST CREW_A(18)
#define CREW_AWORST CREW_A(3)

struct Asteroid {
  Transform transform;
  uint64_t mineral_source;
  unsigned implode : 1;
  unsigned deplete : 1;
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
  Blackboard bb;

  int id;
  int kind = kPlayerControlled;

  int ship = 0;
  uint8_t acurrent[CREWA_MAX];
  uint8_t aknown_min[CREWA_MAX] = CREW_AWORST;
  uint8_t aknown_max[CREWA_MAX] = CREW_ABEST;

  // Bit Fields
  unsigned spacesuit : 1;
  unsigned inspace : 1;
  unsigned uaction : 3;
};

constexpr unsigned kUnitBits = 5;
DECLARE_GAME_TYPE_WITH_ID(Unit, 1 << kUnitBits);
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
  uint64_t running;
  unsigned ftl_frame : 6;
};
DECLARE_GAME_TYPE(Ship, 1);

struct Missile {
  Transform transform;
  v2i tile_hit;
  signed y_velocity : 3 + 1;
  unsigned explode_frame : 6;
};
DECLARE_GAME_TYPE(Missile, 8);

struct Module {
  unsigned cx : 5;
  unsigned cy : 5;
  unsigned mod_power : 1;
  unsigned mod_engine : 1;
  unsigned mod_mine : 1;
  unsigned mod_turret : 1;
};
DECLARE_GAME_TYPE(Module, 32);

struct Command {
  UnitAction type;
  v3f destination;
  uint64_t unit : kUnitBits;
};
DECLARE_GAME_QUEUE(Command, 16);

struct Player {
  Camera camera;  // This assumes fixed dimension window
  v2f mouse;      // Cursor position in world
  unsigned mod_placement : 1;
};
DECLARE_GAME_TYPE(Player, MAX_PLAYER);

constexpr unsigned kNotifyAgeBits = 5;
struct Notify {
  v3f position;
  unsigned age : kNotifyAgeBits;
};
DECLARE_GAME_TYPE(Notify, 4);

struct Consumable {
  unsigned cx : 5;
  unsigned cy : 5;
  unsigned cryo_chamber : 1;
  uint32_t minerals : 6;
};
DECLARE_GAME_TYPE(Consumable, 16);
