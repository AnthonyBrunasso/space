#pragma once

namespace mood {

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
        } break;
        case kSpawnerSnail: {
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
