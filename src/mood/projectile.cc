#pragma once

namespace mood {

u32 PlayerId(); // defined in character.cc

void
ProjectileCreate(v2f start, v2f dir, u32 from_entity,
                 const WeaponComponent& weapon)
{
  ecs::Entity* creator_entity = ecs::FindEntity(from_entity);
  v2f start_offset = {};
  r32 angle = atan2(dir.y, dir.x) * 180.f / PI;
  v2f size;
  physics::Particle2d* particle = nullptr;
  ecs::Entity* pentity = ecs::UseEntity();
  ProjectileComponent* projectile = AssignProjectileComponent(pentity);
  switch (weapon.projectile_type) {
    case kProjectileBullet:
    case kProjectileLaser: {
      particle =
          physics::CreateParticle2d(start, v2f(10.5f, 1.2f), pentity->id);
      particle->rotation = angle;
      SBIT(particle->flags, physics::kParticleIgnoreGravity);
      SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(particle->flags, physics::kParticleIgnoreDamping);
      projectile->ttl = weapon.projectile_ttl;
      projectile->speed = weapon.projectile_speed;
    } break;
    case kProjectileGrenade: {
      physics::Particle2d* particle_creator = ecs::GetParticle(creator_entity);
      v2f start_offset = {};
      if (particle_creator) {
        start_offset = v2f((particle_creator->dims.x / 2.f) * dir.x, 0.f);
      }
      particle =
          physics::CreateParticle2d(start, v2f(10.5f, 1.2f), pentity->id);
      particle->rotation = 0.f;
      SBIT(particle->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(particle->flags, physics::kParticleIgnoreDamping);
      projectile->ttl = weapon.projectile_ttl; 
      projectile->speed = weapon.projectile_speed;
    } break;
    default: {
      assert(!"Unknown projectile type...");
    } break;
  }
  ecs::AssignPhysicsComponent(pentity)->particle_id = particle->id;
  dir += v2f(0.f, math::Random(-0.05f, 0.05f));
  projectile->dir = dir;
  projectile->projectile_type = weapon.projectile_type;
  projectile->from_entity = from_entity;
  if (from_entity != PlayerId()) {
    SBIT(particle->collision_mask, kCollisionMaskAI);
  }
}

void
ProjectileUpdate()
{
  ECS_ITR2(itr, kPhysicsComponent, kProjectileComponent);
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

    --p->ttl;
    if (!p->ttl) {
      ecs::AssignDeathComponent(itr.e);
    }
  }
}

}
