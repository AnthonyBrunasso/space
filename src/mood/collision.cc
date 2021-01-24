#pragma once

namespace mood {

void
__CharacterProjectileCollision(CharacterComponent* character,
                               ProjectileComponent* projectile)
{
  assert(character);
  assert(projectile);
  // If the particle created the projectile - ignore the collision.
  if (projectile->from_entity == character->entity_id) return;
  ecs::AssignDeathComponent(projectile->entity_id);
  character->health -= projectile->damage;
}

void
__ProjectileParticleCollision(ProjectileComponent* projectile,
                              physics::Particle2d* particle,
                              physics::BP2dCollision* c)
{
  assert(projectile);
  assert(particle);
  ecs::Entity* projectile_ent = ecs::FindEntity(projectile->entity_id);
  ecs::AssignDeathComponent(projectile_ent);
  physics::Particle2d* projectile_particle = physics::FindParticle2d(
      ecs::GetPhysicsComponent(projectile_ent)->particle_id);
  // Spawn effect flying off in opposite direction.
  if (projectile_particle) {
    v2f dir = -math::Normalize(projectile_particle->velocity);
    for (int i = 0; i < 4; ++i) {
      physics::Particle2d* ep =
          physics::CreateParticle2d(projectile_particle->position + dir * 1.f,
                                    v2f(kParticleWidth, kParticleHeight));
      SBIT(ep->collision_mask, kCollisionMaskCharacter);
      v2f fdir = Rotate(dir, math::Random(-25.f, 25.f));
      ep->force = fdir * math::Random(3000.f, 15000.f);
      ep->ttl = kParticleTTL;
      SBIT(ep->user_flags, kParticleSpark);
    }
  }
}

void
__PlayerObstacleCollision(CharacterComponent* character,
                          ObstacleComponent* obstacle)
{
  if (obstacle->obstacle_type == kObstacleBoost) {
    physics::Particle2d* p =
        ecs::GetParticle(ecs::FindEntity(character->entity_id));
    p->force.y = kObstacleBoostForce;
  }
}

void
__CharacterDamageCollision(CharacterComponent* character, DamageComponent* damage) {
  if (character->entity_id == damage->from_entity) return;
  character->health -= damage->damage;
}
#if 0
void
__PlayerCharacterCollision(Character* player, Character* character)
{
  if (util::FrameCooldownReady(&kCharacter.player_invulnerable)) {
    player->health -= 1.f;
    util::FrameCooldownReset(&kCharacter.player_invulnerable);
  }
}
#endif

void
CollisionUpdate()
{
#define GET_COMP(T, t, f, e1, e2) \
  T* t = nullptr;                 \
  if (e1 && e1->Has(f)) {         \
    t = ecs::Get##T(e1);          \
  } else if (e2 && e2->Has(f)) {  \
    t = ecs::Get##T(e2);          \
  }

#define GET_COMBO(T1, t1, f1, T2, t2, f2, e1, e2) \
  GET_COMP(T1, t1, f1, e1, e2)                    \
  GET_COMP(T2, t2, f2, e1, e2)

  for (u32 i = 0; i < physics::kUsedBP2dCollision; ++i) {
    physics::BP2dCollision* c = &physics::kBP2dCollision[i];
    ecs::Entity* e1 = ecs::FindEntity(c->p1->entity_id);
    ecs::Entity* e2 = ecs::FindEntity(c->p2->entity_id);

    if (c->p1->collision_mask & c->p2->collision_mask) continue;

    if (e1 && e2) {
      {
        GET_COMBO(CharacterComponent, c, kCharacterComponent,
                  ProjectileComponent, p, kProjectileComponent,
                  e1, e2);
        if (c && p) {
          __CharacterProjectileCollision(c, p);
          continue;
        }
      }

      {
        GET_COMBO(CharacterComponent, c, kCharacterComponent,
                  ObstacleComponent, o, kObstacleComponent,
                  e1, e2);
        if (c && o) {
          __PlayerObstacleCollision(c, o);
        }
      }

      {
        GET_COMBO(CharacterComponent, c, kCharacterComponent,
                  DamageComponent, d, kDamageComponent,
                  e1, e2);
        if (c && d) {
          __CharacterDamageCollision(c, d);
        }
      }
    }

    if (((e1 && e1->Has(kProjectileComponent)) ||
         (e2 && e2->Has(kProjectileComponent))) &&
        (c->p1->inverse_mass == 0.f || c->p2->inverse_mass == 0.f)) {
      GET_COMP(ProjectileComponent, projectile, kProjectileComponent, e1, e2);
      physics::Particle2d* particle = nullptr;
      if (projectile->entity_id == c->p1->entity_id) {
        particle = c->p2;
      } else { particle = c->p1; }
      if (projectile && particle) {
        __ProjectileParticleCollision(projectile, particle, c);
        continue;
      }
    }
  }
}

}
