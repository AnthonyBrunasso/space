#pragma once

namespace mood {

const char*
SpawnerName(SpawnerType type)
{
  switch (type) {
    case kSpawnerPlayer: return "player";
    case kSpawnerSnail: return "snail";
    case kSpawnerFlying: return "flying";
    default: return "unknown";
  }
}

void
SpawnerCreate(v2f pos, SpawnerType type)
{
  switch (type) {
    case kSpawnerFlying:
    case kSpawnerPlayer:
    case kSpawnerSnail: {
      ecs::Entity* entity = ecs::UseEntity();
      SpawnerComponent* spawner = ecs::AssignSpawnerComponent(entity);
      physics::Particle2d* p = physics::CreateParticle2d(
          pos, v2f(5.f, 5.f), entity->id);
      ecs::AssignPhysicsComponent(entity)->particle_id = p->id;
      spawner->spawner_type = type;
      SBIT(p->flags, physics::kParticleIgnoreGravity);
      SBIT(p->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(p->user_flags, kParticleSpawner);
      if (type == kSpawnerFlying) {
        spawner->spawn_to_count = UINT32_MAX;
        spawner->cooldown.frame = 300;
      }
      util::FrameCooldownInitialize(&spawner->cooldown);
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
  ECS_ITR1(itr, kSpawnerComponent);
  while (itr.Next()) {
    SpawnerComponent* s = itr.c.spawner;
    physics::Particle2d* p = GetParticle(itr.e);
    if (!util::FrameCooldownReady(&s->cooldown)) continue;
    if (s->spawn_count < s->spawn_to_count) {
      switch (s->spawner_type) {
        case kSpawnerPlayer: {
          if (!Player()) PlayerCreate(p->position);
        } break;
        case kSpawnerSnail: {
          AICreate(p->position, v2f(kEnemySnailWidth, kEnemySnailHeight),
                   kBehaviorSimple);
        } break;
        case kSpawnerFlying: {
          AICreate(p->position + v2f(math::Random(-1000.f, 1000.f),
                                     math::Random(100.f, 200.f)),
                   v2f(15.f, 15.f), kBehaviorSimpleFlying);
        } break;
        default: {
          printf("%s Unknown spawner type.", __FUNCTION__);
          return;
        } break;
      }
      ++s->spawn_count;
      util::FrameCooldownReset(&s->cooldown);
    }
  }
}

}
