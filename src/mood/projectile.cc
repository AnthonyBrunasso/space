#pragma once

#include "entity.cc"

namespace mood {

void
ProjectileCreate(v2f start, v2f dir, u32 from_entity, ProjectileType type)
{
  Entity* entity_creator = FindEntity(from_entity);
  v2f start_offset = {};
  r32 angle = atan2(dir.y, dir.x) * 180.f / PI;
  v2f size;
  physics::Particle2d* particle = nullptr;
  ecs::Entity* pentity = ecs::UseEntity();
  ProjectileComponent* projectile = AssignProjectileComponent(pentity);
  switch (type) {
    case kProjectileBullet:
    case kProjectileLaser: {
      particle = physics::CreateParticle2d(start, v2f(10.5f, 1.2f));
      particle->rotation = angle;
      SBIT(particle->flags, physics::kParticleIgnoreGravity);
      SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(particle->flags, physics::kParticleIgnoreDamping);
      projectile->updates_to_live = 50;
      projectile->speed = kProjectileSpeed;
    } break;
    case kProjectileGrenade: {
      physics::Particle2d* particle_creator =
          physics::FindParticle2d(entity_creator->particle_id);
      v2f start_offset = {};
      if (particle_creator) {
        start_offset = v2f((particle_creator->dims.x / 2.f) * dir.x, 0.f);
      }
      particle = physics::CreateParticle2d(start, v2f(10.5f, 1.2f));
      //particle->force = dir * 50000.f;
      particle->rotation = 0.f;
      SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(particle->flags, physics::kParticleIgnoreDamping);
      projectile->updates_to_live = 300;
      projectile->speed = kGrenadeSpeed;
    } break;
    default: {
      assert(!"Unknown projectile type...");
    } break;
  };
  ecs::AssignPhysicsComponent(pentity)->particle_id = particle->id;
  particle->entity_id = pentity->id;
  dir += v2f(0.f, math::Random(-0.05f, 0.05f));
  projectile->dir = dir;
  projectile->projectile_type = type;
  projectile->from_entity = from_entity;
}

void
ProjectileUpdate()
{
  ecs::EntityItr<2> itr(kPhysicsComponent, kProjectileComponent);
  while (itr.Next()) {
    physics::Particle2d* particle =
        physics::FindParticle2d(itr.c.physics->particle_id);
    ProjectileComponent* p = itr.c.projectile;
    if (particle) {
      switch (p->projectile_type) {
        case kProjectileLaser: {
        } break;
        case kProjectileBullet: {
          v2f delta = p->dir * p->speed;
          particle->velocity = delta;
        } break;
        case kProjectileGrenade: {
          v2f delta = p->dir * p->speed;
          particle->velocity = delta;
          particle->velocity.y -= physics::kPhysics.gravity * kFrameDelta;
        } break;
        default: break;
      }
    }

    //--p->updates_to_live;
    //if (!p->updates_to_live) {
    //  SetDestroyFlag(p);
    //}
  }
}

}
