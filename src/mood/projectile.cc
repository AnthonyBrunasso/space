#pragma once

#include "entity.cc"

namespace mood {

void
ProjectileCreate(v2f start, v2f dir, u32 from_entity, ProjectileType type)
{
  Entity* entity_creator = FindEntity(from_entity);
  v2f start_offset = {};
  if (entity_creator) {
    physics::Particle2d* particle_creator =
        physics::FindParticle2d(entity_creator->particle_id);
    if (particle_creator) {
      start_offset.x += (particle_creator->dims.x / 2.f) * dir.x;
    }
  }
  Projectile* projectile =
      UseEntityProjectile(start + start_offset, v2f(1.0f, .2f));
  physics::Particle2d* particle = FindParticle(projectile);
  switch (type) {
    case kProjectileLaser: {
      SBIT(particle->flags, physics::kParticleIgnoreGravity);
      SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(particle->flags, physics::kParticleIgnoreDamping);
    } break;
    default: break;
  };
  projectile->dir = dir;
  projectile->projectile_type = type;
  projectile->updates_to_live = 50;
  projectile->from_entity = from_entity;
}

void
ProjectileUpdate()
{
  FOR_EACH_ENTITY(Projectile, p, {
    // Run projectile updates logic here. 
    physics::Particle2d* particle = FindParticle(p);
    if (particle) {
      v2f delta = p->dir * 100.f;
      particle->velocity = delta;
    }

    --p->updates_to_live;
    if (!p->updates_to_live) {
      SetDestroyFlag(p);
    }
  });
}

}
