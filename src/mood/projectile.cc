#pragma once

#include "entity.cc"

namespace mood {

void
ProjectileCreate(v2f start, v2f dir, u32 from_entity, ProjectileType type)
{
  Entity* entity_creator = FindEntity(from_entity);
  v2f start_offset = {};
  physics::Particle2d* particle_creator = nullptr;
  if (entity_creator) {
    particle_creator = physics::FindParticle2d(entity_creator->particle_id);
    if (particle_creator) {
      //start_offset.x += (particle_creator->dims.x / 2.f) * dir.x;
    }
  }
  Projectile* projectile =
      UseEntityProjectile(start + start_offset, v2f(10.5f, 1.2f));
  physics::Particle2d* particle = FindParticle(projectile);
  r32 angle = atan2(dir.y, dir.x) * 180.f / PI;
  particle->rotation = angle;
  switch (type) {
    case kProjectileBullet:
    case kProjectileLaser: {
      SBIT(particle->flags, physics::kParticleIgnoreGravity);
      SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(particle->flags, physics::kParticleIgnoreDamping);
    } break;
    default: break;
  };
  dir += v2f(0.f, math::Random(-0.05f, 0.05f));
  projectile->dir = dir;
  projectile->projectile_type = type;
  projectile->updates_to_live = 50;
  projectile->from_entity = from_entity;
  projectile->speed = kProjectileSpeed;
}

void
ProjectileUpdate()
{
  FOR_EACH_ENTITY(Projectile, p, {
    physics::Particle2d* particle = FindParticle(p);
    if (particle) {
      switch (p->projectile_type) {
        case kProjectileLaser: {
        } break;
        case kProjectileBullet: {
          v2f delta = p->dir * p->speed;
          particle->velocity = delta;
        } break;
        default: break;
      }
    }

    --p->updates_to_live;
    if (!p->updates_to_live) {
      SetDestroyFlag(p);
    }
  });
}

}
