#pragma once

namespace mood {

struct Player {
  u32 id = 0;
  // Cooldown that dictates whether the player can boost.
  util::FrameCooldown boost_cooldown;
  // Cooldown that makes player invulnerable.
  util::FrameCooldown player_invulnerable;
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
}

}
