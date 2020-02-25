
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

// id == -1 is reserved for invalid references
#define DECLARE_GAME_TYPE_WITH_ID(type, max_count)                            \
  DECLARE_ARRAY(type, max_count)                                              \
  static uint32_t kInvalid##type = 0;                                         \
  static uint32_t kAutoIncrementId##type = 1;                                 \
                                                                              \
  type* UseId##type()                                                         \
  {                                                                           \
    type* t = Use##type();                                                    \
    if (!t) return nullptr;                                                   \
    t->id = kAutoIncrementId##type;                                           \
    kAutoIncrementId##type += (kAutoIncrementId##type == kInvalid##type) + 1; \
    return t;                                                                 \
  }                                                                           \
                                                                              \
  type* Find##type(uint32_t id)                                               \
  {                                                                           \
    for (int i = 0; i < kUsed##type; ++i) {                                   \
      if (k##type[i].id == id) return &k##type[i];                            \
    }                                                                         \
    return nullptr;                                                           \
  }                                                                           \
  static EntityRegistry kInit##type(k##type, &kZero##type, &kUsed##type,      \
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
  kOperator,
  kMilitary,
};
enum ModuleKind {
  kModPower = 0,
  kModEngine,
  kModMine,
  kModTurret,
  kModCount,
};
constexpr unsigned kModBits = 2;
enum UnitAlliance { kCrew, kEnemy };
enum ShipAiGoals {
  kShipAiSpawnPod,
  kShipAiPowerSurge,
  kShipAiGoals = 64,
};
enum UnitBbEntry {
  kUnitTarget = 0,
  kUnitDestination = 1,
  kUnitBbEntryMax = 2,
};
// TODO (AN): Document/test priority behaviors
enum PodAiGoals {
  kPodAiLostControl,
  kPodAiUnmanned,
  kPodAiGather,
  kPodAiUnload,
  kPodAiReturn,
  kPodAiApproach,
  kPodAiDisembark,
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
  uint64_t ship_index = UINT64_MAX;
  v2f goal;
  v2f last_heading;
  uint64_t mineral;
};
DECLARE_GAME_TYPE(Pod, 8);

struct Unit {
  Transform transform;
  Blackboard bb;

  uint32_t id;
  int kind;
  UnitAlliance alliance = kCrew;
  uint64_t ship_index = UINT64_MAX;

  // Maybe need a weapon type?
  float attack_radius = 100.0f;
  int attack_frame;
  // Can attack every N frames.
  int attack_cooldown = 60;
  float health = 10.0f;

  uint8_t acurrent[CREWA_MAX];
  uint8_t aknown_min[CREWA_MAX] = CREW_AWORST;
  uint8_t aknown_max[CREWA_MAX] = CREW_ABEST;

  // Bit Fields
  unsigned spacesuit : 1;
  unsigned inspace : 1;
  unsigned uaction : 3;
  unsigned mskill : kModBits;
};

DECLARE_GAME_TYPE_WITH_ID(Unit, 16);
struct Ship {
  uint64_t think_flags = 0;
  uint64_t operate_flags = 0;
  uint64_t danger = 0;
  uint64_t grid_index = UINT64_MAX;
  float sys[kModCount];
  float used_power;
  float power_delta;
  uint64_t level;
  uint64_t pod_capacity;
  unsigned ftl_frame : 6;
};
DECLARE_GAME_TYPE(Ship, 2);
struct Resource {
  uint64_t mineral = 0;
  uint64_t frame = 0;
};
DECLARE_GAME_TYPE(Resource, 1);

struct Missile {
  Transform transform;
  v2i tile_hit;
  signed y_velocity : 3 + 1;
  unsigned explode_frame : 6;
};
DECLARE_GAME_TYPE(Missile, 8);

struct Projectile {
  v3f start;
  v3f end;
  v3f dir;
  float speed;
  // Duration in frames.
  int duration;
};
DECLARE_GAME_TYPE(Projectile, 16);

struct Module {
  uint64_t ship_index = UINT64_MAX;
  unsigned cx : 5;
  unsigned cy : 5;
  unsigned mkind : kModBits;
};
DECLARE_GAME_TYPE(Module, 32);

struct Command {
  UnitAction type;
  v3f destination;
  uint32_t unit_id;
};
DECLARE_GAME_QUEUE(Command, 16);

struct Player {
  Camera camera;  // Assumes fixed dimension window
  v2f world_mouse;
  unsigned mod_placement : kModBits;
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

struct Selection {
  uint32_t unit_id;
};
DECLARE_GAME_TYPE(Selection, 16);

constexpr int kMapWidth = 32;
constexpr int kMapHeight = 32;
struct Tile {
  unsigned cx : 8;
  unsigned cy : 8;
  unsigned blocked : 1;
  unsigned nooxygen : 1;
  unsigned shroud : 1;
  unsigned explored : 1;
  unsigned exterior : 1;
};
struct Grid {
  Transform transform;
  Tile tilemap[kMapHeight][kMapWidth];
};
DECLARE_GAME_TYPE(Grid, 2);

