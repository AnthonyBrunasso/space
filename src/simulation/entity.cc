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
#define DECLARE_GAME_TYPE_WITH_ID(type, max_count)                       \
  DECLARE_ID_ARRAY(type, max_count)                                      \
  static EntityRegistry kInit##type(k##type, &kZero##type, &kUsed##type, \
                                    max_count, sizeof(type));

#define DECLARE_GAME_QUEUE(type, count) DECLARE_QUEUE(type, count)

// Use with DECLARE_GAME_TYPE_WITH_ID(Entity, ...)
#define DECLARE_GAME_ENTITY(type)                                             \
  type* UseEntity##type()                                                     \
  {                                                                           \
    Entity* e = UseEntity();                                                  \
    if (!e) return nullptr;                                                   \
    type t;                                                                   \
    t.id = kAutoIncrementIdEntity;                                            \
    kAutoIncrementIdEntity += (kAutoIncrementIdEntity == kInvalidEntity) + 1; \
    memcpy(e, &t, sizeof(t));                                                 \
    return (type*)e;                                                          \
  }

// Global game state that is local information
static uint64_t kPlayerCount;
static uint64_t kPlayerIndex;

constexpr uint64_t kInvalidIndex = UINT64_MAX;

// Common Flags
enum UnitAction {
  kUaNone = 0,
  kUaMove,
  kUaOperate,
  kUaAttack,
  kUaAttackMove,
  kUaBuild,
};
enum ModuleKind {
  kModPower = 0,
  kModEngine,
  kModTurret,
  kModMine,
  kModBarrack,
  kModMedbay,
  kModWarp,
  kModCount,
};
enum UnitKind {
  kOperator,
  kAlien,
};
enum HudMode {
  kHudSelection = 0,
  kHudModule,
  kHudAttackMove,
  kHudDefault = kHudSelection,
};
enum WeaponKind {
  kWeaponLaser = 0,
  kWeaponBullet,
  kWeaponMiningLaser,
  kWeaponCount,
};
constexpr unsigned kHudModeBits = 2;
constexpr unsigned kModBits = 3;
enum Alliance { kCrew = 0, kEnemy, kAllianceCount };
enum ShipAiGoals {
  kShipAiSpawnPod,
  kShipAiPowerSurge,
  kShipAiGoals = 64,
};
constexpr int kUnitBehaviorSimple = 1;
constexpr int kUnitBehaviorAttackWhenDiscovered = 2;
enum UnitBbEntry {
  kUnitTarget = 0,
  kUnitDestination = 1,
  kUnitAttackDestination = 2,
  kUnitBehavior = 3,
  kUnitAttacker = 4,
  kUnitBbEntryMax = 5,
};

// Common Structures
struct Transform {
  v3f position;
  v3f scale = v3f(1.f, 1.f, 1.f);
};

const char* const deck_name[] = {
    "Cargo",
    "Bridge",
    "Observation",
};
constexpr uint64_t kMaxDeck = ARRAY_LENGTH(deck_name);
constexpr uint64_t kLastDeck = kMaxDeck - 1;

struct Asteroid {
  Transform transform;
  float mineral_source;
  float deplete;
  bool implode;
};
DECLARE_GAME_TYPE(Asteroid, 8);

constexpr uint64_t kNonPlayerId = UINT64_MAX;
constexpr unsigned kNotifyAgeBits = 5;

struct Ship {
  uint64_t grid_index = UINT64_MAX;
  uint64_t level;
  uint64_t deck = 1;
  uint64_t pod_capacity;
  unsigned ftl_frame : 6;
  unsigned engine_animation : 4;
  uint64_t PADDING : 22;
};
DECLARE_GAME_TYPE(Ship, 2);

struct Missile {
  Transform transform;
  v2i tile_hit;
  signed y_velocity : 3 + 1;
  unsigned explode_frame : 6;
  unsigned PADDING : 22;
};
DECLARE_GAME_TYPE(Missile, 8);

struct Projectile {
  v3f start;
  v3f end;
  WeaponKind wkind;
  // Frame count since creation
  unsigned frame;
  // Frame duration
  int duration;
  uint32_t target_id;
};
DECLARE_GAME_TYPE(Projectile, 128);

#define COMMON_MEMBER_DECL \
  v3f position;            \
  v3f scale;               \
  v3f bounds;              \
  uint32_t id;             \
  uint64_t control;        \
  uint64_t ship_index;     \
  uint64_t player_id;      \
  int type

enum EntityEnum {
  kEeUnit = 0,
  kEeModule = 1,
};

struct Unit {
  COMMON_MEMBER_DECL = kEeUnit;
  Blackboard bb;
  UnitKind kind;
  Alliance alliance = kCrew;

  // Maybe need a weapon type?
  float attack_radius = 100.0f;
  int attack_frame;
  // Can attack every N frames.
  int attack_cooldown = 60;
  WeaponKind weapon_kind = kWeaponLaser;
  float health = 5.0f;
  float max_health = 5.0f;
  float speed = 1.f;

  // Bit Fields
  unsigned dead : 1;
  unsigned spacesuit : 1;
  unsigned inspace : 1;
  unsigned uaction : 4;
  unsigned persistent_uaction : 3;
  unsigned mskill : kModBits;
  unsigned notify : kNotifyAgeBits;
  uint64_t PADDING : 13;
};

constexpr int kTrainIdle = -1;

struct Module {
  COMMON_MEMBER_DECL = kEeModule;
  int frames_to_build = 200;
  int frames_building = 0;
  int frames_to_train = 800;
  int frames_training = kTrainIdle;
  ModuleKind mkind;
  v2i tile;
  bool enabled = true;
};

union Entity {
  struct {
    COMMON_MEMBER_DECL;
  };
  Unit unit;
  Module module;

  Entity() : type(UINT32_MAX)
  {
  }
};
#define MAX_ENTITY 128
DECLARE_GAME_TYPE_WITH_ID(Entity, MAX_ENTITY);
DECLARE_GAME_ENTITY(Unit);
DECLARE_GAME_ENTITY(Module);

#define FindUnit(x) ((Unit*)FindEntity(x))
#define ZeroEntity(x) (*(Entity*)x = kZeroEntity)

struct Command {
  UnitAction type;
  v3f destination;
  uint32_t unit_id;
  unsigned control : MAX_PLAYER;
  uint64_t PADDING : 30;
};
DECLARE_GAME_QUEUE(Command, 16);

struct Player {
  Camera camera;  // Assumes fixed dimension window
  v3f selection_start;
  v3f world_mouse;
  ModuleKind mod_placement;
  uint64_t ship_index;
  bool admin_menu;
  bool scenario_menu;
  bool feature_menu;
  bool god_mode;
  float mineral = 0;
  uint64_t level = 1;
  unsigned hud_mode : kHudModeBits;
  unsigned local : 1;
  uint64_t PADDING : 58;
};
DECLARE_GAME_TYPE(Player, MAX_PLAYER);

struct Consumable {
  uint64_t ship_index = UINT64_MAX;
  unsigned cx : 5;
  unsigned cy : 5;
  unsigned cryo_chamber : 1;
  uint32_t minerals : 6;
  uint64_t PADDING : 47;
};
DECLARE_GAME_TYPE(Consumable, 32);

constexpr int kMapWidth = 64;
constexpr int kMapHeight = 64;
struct Tile {
  unsigned cx : 8;
  unsigned cy : 8;
  unsigned blocked : 1;
  unsigned nooxygen : 1;
  unsigned shroud : 1;
  unsigned explored : 1;
  unsigned exterior : 1;
  unsigned PADDING : 11;
};
struct Grid {
  Transform transform;
  Tile tilemap[kMapHeight][kMapWidth];
  bool fog = true;
};
DECLARE_GAME_TYPE(Grid, 2);

constexpr int kMaxInvasionCount = 10;

// TODO: Find a better place for this. Controls the max units allowed in an
// invasion and increments up to kMaxInvasionCount.
static int kMaxThisInvasion = 2;

struct Invasion {
  Transform transform;
  // Units countained in the invasion.
  uint32_t unit_id[kMaxInvasionCount];
  int unit_count;

  bool docked = false;
  v2i docked_tile;
};
DECLARE_GAME_TYPE(Invasion, 2);
