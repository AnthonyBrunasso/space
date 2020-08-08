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
            //Character* player =
            //    UseEntityCharacter(p->position,
            //                       v2f(kPlayerWidth, kPlayerHeight));
            //physics::Particle2d* particle = FindParticle(player);
            ecs::Entity* player_entity = ecs::UseEntity();
            printf("Creating player %u\n", player_entity->id);
            kCharacter.player_id = player_entity->id;
            CharacterComponent* player_comp =
                ecs::AssignCharacterComponent(player_entity);
            PhysicsComponent* physics_comp =
                ecs::AssignPhysicsComponent(player_entity);
            physics::Particle2d* particle =  physics::CreateParticle2d(
                p->position, v2f(kPlayerWidth, kPlayerHeight));
            physics_comp->particle_id = particle->id;
            particle->collision_mask = kCollisionMaskCharacter;
            particle->damping = 0.005f;
            player_comp->double_jump_cooldown.frame = 15;
            util::FrameCooldownInitialize(&player_comp->double_jump_cooldown);
            player_comp->weapon_cooldown.frame = 10;
            util::FrameCooldownInitialize(&player_comp->weapon_cooldown);
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
