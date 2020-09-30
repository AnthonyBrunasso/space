#pragma once

namespace mood {

enum PlayerWeapon {
  kPlayerWeaponSword = 0,
  kPlayerWeaponCount = 1,
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
  PlayerWeapon current_weapon = kPlayerWeaponSword;
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
  if (!ent) return nullptr;
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
  AnimComponent* anim_comp =
      ecs::AssignAnimComponent(player_entity);
  MeleeWeaponComponent* weapon_comp =
      ecs::AssignMeleeWeaponComponent(player_entity);
  AnimInitAdventurerFSM(&anim_comp->fsm);
  physics::Particle2d* particle =  physics::CreateParticle2d(
      position, v2f(kPlayerWidth, kPlayerHeight),
      player_entity->id);
  physics_comp->particle_id = particle->id;
  SBIT(particle->collision_mask, kCollisionMaskCharacter);
  particle->damping = 0.005f;
  player_comp->double_jump_cooldown.frame = 15;
  util::FrameCooldownInitialize(&player_comp->double_jump_cooldown);
  PlayerSetWeapon(kPlayerWeaponSword);
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

void
PlayerAttack()
{
  ecs::Entity* player = Player();
  if (!player) return;
  //AnimComponent* anim = ecs::GetAnimComponent(player);
  //if (!anim->fsm.CanInterrupt()) return;
  CharacterComponent* character = ecs::GetCharacterComponent(player);
  SBIT(character->character_flags, kCharacterAttackMelee);
}

void
PlayerStopAttack()
{
  ecs::Entity* player = Player();
  if (!player) return;
  CharacterComponent* character = ecs::GetCharacterComponent(player);
  CBIT(character->character_flags, kCharacterAttackMelee);
}

void
PlayerJump()
{
  ecs::Entity* player = Player();
  if (!player) return;
  //AnimComponent* anim = ecs::GetAnimComponent(player);
  //if (!anim->fsm.CanInterrupt()) return;
  CharacterComponent* character = ecs::GetCharacterComponent(player);
  SBIT(character->character_flags, kCharacterJump);
}

void
PlayerStopJump()
{
  ecs::Entity* player = Player();
  if (!player) return;
  CharacterComponent* character = ecs::GetCharacterComponent(player);
  CBIT(character->character_flags, kCharacterJump);
}

void
PlayerMove(v2f move_dir, r32 move_multiplier)
{
  ecs::Entity* player = Player();
  if (!player) return;
  //AnimComponent* anim = ecs::GetAnimComponent(player);
  //if (!anim->fsm.CanInterrupt()) return;
  CharacterComponent* character = ecs::GetCharacterComponent(player);
  character->move_dir = move_dir;
  character->move_multiplier = move_multiplier;
  SBIT(character->character_flags, kCharacterMove);
}

void
PlayerStopMove()
{
  ecs::Entity* player = Player();
  if (!player) return;
  CharacterComponent* character = ecs::GetCharacterComponent(player);
  CBIT(character->character_flags, kCharacterMove);
}

}
