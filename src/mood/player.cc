#pragma once

namespace mood {

enum PlayerWeapon {
  kPlayerWeaponMachinegun = 0,
  kPlayerWeaponShotgun = 1,
  kPlayerWeaponCount = 2,
};

struct Player {
  u32 id = 0;
  // Cooldown that dictates whether the player can boost.
  util::FrameCooldown boost_cooldown;
  // Cooldown that makes player invulnerable.
  util::FrameCooldown player_invulnerable;
  // How long the player must wait to switch a weapon.
  util::FrameCooldown change_weapon_cooldown;
  // Current weapon the player is using.
  PlayerWeapon current_weapon = kPlayerWeaponMachinegun;
};

static Player kPlayer;

ecs::Entity*
Player()
{
  return ecs::FindEntity(kPlayer.id);
}

physics::Particle2d*
PlayerParticle()
{
  ecs::Entity* ent = Player();
  return physics::FindParticle2d(ecs::GetPhysicsComponent(ent)->particle_id);
}

u32
PlayerId()
{
  return kPlayer.id;
}

void
PlayerSetWeapon(PlayerWeapon weapon_type)
{
  ecs::Entity* player_entity = Player();
  WeaponComponent* weapon = ecs::GetWeaponComponent(player_entity);
  if (!weapon) {
    weapon = ecs::AssignWeaponComponent(player_entity);
  }
  switch (weapon_type) {
    case kPlayerWeaponMachinegun: {
      weapon->projectile_type = kProjectileBullet;
      weapon->random_aim_offset = 5.f;
      weapon->projectile_ttl = kProjectileTtl;
      weapon->num_projectile = 1;
      weapon->cooldown.frame = 10;
      util::FrameCooldownInitialize(&weapon->cooldown);
    } break;
    case kPlayerWeaponShotgun: {
      weapon->projectile_type = kProjectileBullet;
      weapon->random_aim_offset = 10.f;
      weapon->projectile_ttl = kProjectileTtl;
      weapon->num_projectile = 10;
      weapon->cooldown.frame = 40;
    } break;
    default: assert(!"Undefined weapon type.");
  }
  kPlayer.current_weapon = weapon_type;
}

bool
IsPlayer(ecs::Entity* ent)
{
  return kPlayer.id && kPlayer.id == ent->id;
}

void
PlayerInitialize()
{
  kPlayer.boost_cooldown.frame = 60;
  util::FrameCooldownInitialize(&kPlayer.boost_cooldown);
  kPlayer.player_invulnerable.frame = 20;
  util::FrameCooldownInitialize(&kPlayer.player_invulnerable);
  kPlayer.change_weapon_cooldown.frame = 60;
  util::FrameCooldownInitialize(&kPlayer.change_weapon_cooldown);
}

void
PlayerCreate(v2f position)
{
  ecs::Entity* player_entity = ecs::UseEntity();
  kPlayer.id = player_entity->id;
  CharacterComponent* player_comp =
      ecs::AssignCharacterComponent(player_entity);
  PhysicsComponent* physics_comp =
      ecs::AssignPhysicsComponent(player_entity);
  physics::Particle2d* particle =  physics::CreateParticle2d(
      position, v2f(kPlayerWidth, kPlayerHeight),
      player_entity->id);
  physics_comp->particle_id = particle->id;
  SBIT(particle->collision_mask, kCollisionMaskCharacter);
  particle->damping = 0.005f;
  player_comp->double_jump_cooldown.frame = 15;
  util::FrameCooldownInitialize(&player_comp->double_jump_cooldown);
  PlayerSetWeapon(kPlayerWeaponMachinegun);
}

void
PlayerNextWeapon()
{
  if (!util::FrameCooldownReady(&kPlayer.change_weapon_cooldown)) {
    return;
  }
  PlayerSetWeapon(
      (PlayerWeapon)((kPlayer.current_weapon + 1) % kPlayerWeaponCount));
  util::FrameCooldownReset(&kPlayer.change_weapon_cooldown);
}

void
PlayerPrevWeapon()
{
  if (!util::FrameCooldownReady(&kPlayer.change_weapon_cooldown)) {
    return;
  }
  if (kPlayer.current_weapon == 0) {
    PlayerSetWeapon((PlayerWeapon)(kPlayerWeaponCount - 1));
  } else {
    PlayerSetWeapon((PlayerWeapon)(kPlayer.current_weapon - 1));
  }
  util::FrameCooldownReset(&kPlayer.change_weapon_cooldown);
}

}
