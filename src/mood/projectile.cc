#pragma once

#include "entity.cc"

namespace mood {

Projectile*
ProjectileCreate(v2f start, v2f dir, ProjectileType type)
{
  physics::Particle2d* particle = nullptr;
  switch (type) {
    case PROJECTILE_LAZER: {
      particle = physics::CreateParticle2d(start, v2f(.3f, .3f));
      SBIT(particle->flags, physics::kParticleIgnoreGravity);
      SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
    } break;
    default: break;
  };
  if (!particle) return nullptr;
  SBIT(particle->user_flags, kPhysicsProjectile);
  Projectile* projectile = UseProjectile();
  projectile->particle_id = particle->id;
  projectile->dir = dir;
  //printf("Create particle %u\n", particle->id);
  projectile->type = type;
  projectile->updates_to_live = 20;
}

void
ProjectileUpdate()
{
  for (s32 i = 0; i < kUsedProjectile;) {
    Projectile* p = &kProjectile[i];
    
    // Run projectile updates logic here. 
    physics::Particle2d* particle = ProjectileParticle(i);  

    v2f delta = p->dir * 10.f;
    particle->dims += p->dir * 10.f;
    particle->position += delta / 2.f;

    --p->updates_to_live;
    if (!p->updates_to_live) {
      //printf("Delete particle %u\n", p->particle_id);
      // Delete the particle before compressing or we'll delete the wrong
      // particle.
      physics::DeleteParticle2d(p->particle_id);
      CompressProjectile(i);
      continue;
    }
    ++i;
  }
}

}
