#pragma once

namespace mood {

const char*
SpawnerName(SpawnerType type)
{
  switch (type) {
    case kSpawnerPlayer: return "player";
    case kSpawnerSnail: return "snail";
    default: return "unknown";
  }
}

void
SpawnerCreate(v2f pos, SpawnerType type)
{
  switch (type) {
    case kSpawnerPlayer:
    case kSpawnerSnail: {
      Spawner* spawner = UseEntitySpawner(pos, v2f(5.f, 5.f));
      spawner->spawner_type = type;
      physics::Particle2d* p = FindParticle(spawner);
      SBIT(p->flags, physics::kParticleIgnoreGravity);
      SBIT(p->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(p->user_flags, kParticleSpawner);
    } break;
    default: {
      printf("%s Unknown spawner type.", __FUNCTION__);
      return;
    } break;
  }
}

void
SpawnerUpdate()
{
  FOR_EACH_ENTITY_P(Spawner, s, p, {
    if (s->spawn_count < s->spawn_to_count) {
      switch (s->spawner_type) {
        case kSpawnerPlayer: {
          if (!Player()) {
            Character* player =
                UseEntityCharacter(p->position,
                                   v2f(kPlayerWidth, kPlayerHeight));
            physics::Particle2d* particle = FindParticle(player);
            particle->collision_mask = kCollisionMaskCharacter;
            particle->damping = 0.005f;
            kCharacter.player_id = player->id;
            player->double_jump_cooldown.frame = 15;
            util::FrameCooldownInitialize(&player->double_jump_cooldown);
            player->weapon_cooldown.frame = 10;
            util::FrameCooldownInitialize(&player->weapon_cooldown);
          }
        } break;
        case kSpawnerSnail: {
          AICreate(p->position, v2f(kEnemySnailWidth, kEnemySnailHeight),
                   kBehaviorSimple);
        } break;
        default: {
          printf("%s Unknown spawner type.", __FUNCTION__);
          return;
        } break;
      }
      ++s->spawn_count;
    }
  });
}

}
