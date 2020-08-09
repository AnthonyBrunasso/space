#pragma once


namespace mood {

enum TypeId : u64 {
  kPhysicsComponent = 0,
  kAIComponent = 1,
  kCharacterComponent = 2,
  kDeathComponent = 3,
  kProjectileComponent = 4,
  kObstacleComponent = 5,
  kSpawnerComponent = 6,
  kComponentCount = 7,
}; 

struct PhysicsComponent {
  u32 entity_id = 0;
  u32 particle_id = 0;
};

struct AIComponent {
  u32 entity_id = 0;
  Blackboard blackboard;
};

struct CharacterComponent {
  u32 entity_id = 0;
  u32 flags = 0; // TODO: Remove?
  v2f facing = {1.f, 0.f};
  u8 prev_character_flags = 0;
  u8 character_flags = 0;
  // Used for when a character executes an ability.
  u8 ability_flags = 0; // TODO: Make a component
  // Used to show a flashy trail effect when the player executes a boost.
  u8 trail_effect_ttl = 0; // TODO: Make a component
  // Character stats.
  r32 health = 10.f;
  r32 max_health = 10.f;
  // Use if character is aiming.
  v2f aim_dir = {1.f, 0.f};
  // How much the aim should rotate.
  r32 aim_rotate_delta = 0.f;
  // Which frame of the animation this character is on.
  u32 anim_frame = 0;
  // Direction of character movement.
  v2f move_dir = {};
  // Multiplied by character move acceleration.
  r32 move_multiplier = 0.f;
  // Speed at which the character should move.
  r32 move_acceleration = kPlayerAcceleration;
  // How long the character must wait before they can perform a double jump.
  util::FrameCooldown double_jump_cooldown;
  // Cooldown that lets character fire their weapon.
  util::FrameCooldown weapon_cooldown;
};

// If set on an entity will remove it at the end of the current frame.
struct DeathComponent {
  u32 entity_id = 0;
};

struct ProjectileComponent {
  u32 entity_id = 0;
  v2f dir;
  ProjectileType projectile_type;
  // Number of updates the projectile should live for.
  uint64_t updates_to_live = 0;
  // Entity that fired this projectile.
  u32 from_entity = 0; 
  // Entity that fired the projectile.
  r32 speed = 500.f;
};

struct ObstacleComponent {
  u32 entity_id = 0;
  ObstacleType obstacle_type = kObstacleNone;
};

struct SpawnerComponent {
  u32 entity_id = 0;
  SpawnerType spawner_type = kSpawnerNone;
  // How many times the spawner should spawn the given character.
  u32 spawn_to_count = 1;
  // How many times the spawner has spawned the given character.
  u32 spawn_count = 0;
};

}

namespace ecs {

struct Components {
  mood::PhysicsComponent*    physics    = nullptr;
  mood::AIComponent*         ai         = nullptr;
  mood::CharacterComponent*  character  = nullptr;
  mood::DeathComponent*      death      = nullptr;
  mood::ProjectileComponent* projectile = nullptr;
  mood::ObstacleComponent*   obstacle   = nullptr;
  mood::SpawnerComponent*    spawner    = nullptr;
};

}

#define ENTITY_COUNT 2048
#include "ecs/ecs.cc"

namespace ecs {

using namespace mood;

ComponentStorage*
GetComponents(u64 tid)
{
  switch (tid) {
    case kPhysicsComponent: {
      static ecs::ComponentStorage f(
          PHYSICS_PARTICLE_COUNT, sizeof(PhysicsComponent));
      return &f;
    } break;
    case kAIComponent: {
      static ecs::ComponentStorage f(64, sizeof(AIComponent));
      return &f;
    } break;
    case kCharacterComponent: {
      static ecs::ComponentStorage f(128, sizeof(CharacterComponent));
      return &f;
    } break;
    case kDeathComponent: {
      static ecs::ComponentStorage f(64, sizeof(DeathComponent));
      return &f;
    } break;
    case kProjectileComponent: {
      static ecs::ComponentStorage f(256, sizeof(ProjectileComponent));
      return &f;
    } break;
    case kObstacleComponent: {
      static ecs::ComponentStorage f(32, sizeof(ObstacleComponent));
      return &f;
    } break;
    case kSpawnerComponent: {
      static ecs::ComponentStorage f(32, sizeof(SpawnerComponent));
      return &f;
    } break;
    default: {
      assert(!"Unknown component type");
    } break;
  }
  return nullptr;
}

DECLARE_COMPONENT(PhysicsComponent, kPhysicsComponent);
DECLARE_COMPONENT(AIComponent, kAIComponent);
DECLARE_COMPONENT(CharacterComponent, kCharacterComponent);
DECLARE_COMPONENT(DeathComponent, kDeathComponent);
DECLARE_COMPONENT(ProjectileComponent, kProjectileComponent);
DECLARE_COMPONENT(ObstacleComponent, kObstacleComponent);
DECLARE_COMPONENT(SpawnerComponent, kSpawnerComponent);

physics::Particle2d*
GetParticle(ecs::Entity* ent)
{
  return physics::FindParticle2d(ecs::GetPhysicsComponent(ent)->particle_id);
}

template <typename Component>
physics::Particle2d*
GetParticle(Component* c)
{
  ecs::Entity* ent = ecs::FindEntity(c->entity_id);
  return GetParticle(ent);
}

}
