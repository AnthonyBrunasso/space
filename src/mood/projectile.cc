#pragma once

#include "entity.cc"

namespace mood {

void
ProjectileCreate(v2f start, v2f dir, ProjectileType type)
{
  Projectile* projectile = UseEntityProjectile(start, v2f(.3f, .3f));
  physics::Particle2d* particle = GetParticle(projectile);
  switch (type) {
    case kProjectileLaser: {
      SBIT(particle->flags, physics::kParticleIgnoreGravity);
      SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
    } break;
    default: break;
  };
  projectile->dir = dir;
  projectile->projectile_type = type;
  projectile->updates_to_live = 20;
}

void
ProjectileUpdate()
{
  FOR_EACH_ENTITY(Projectile, p, {
    // Run projectile updates logic here. 
    physics::Particle2d* particle = GetParticle(p);

    v2f delta = p->dir * 10.f;
    particle->dims += p->dir * 10.f;
    particle->position += delta / 2.f;

    --p->updates_to_live;
    if (!p->updates_to_live) {
      SetDestroyFlag(p);
    }
  });
}

}
