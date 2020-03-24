#pragma once

#include "entity.cc"
#include "util.cc"

namespace simulation
{
constexpr uint32_t kLaserDuration = 60;
constexpr float kHitDsq = 13.f * 13.f;
constexpr float kLaserDamage = 0.01f;
constexpr float kBulletDamage = 0.50f;

void
ProjectileCreate(v3f target, v3f source, float proximity, uint32_t duration,
                 WeaponKind kind)
{
  float radian = (float)(rand() % 360) * PI / 180.0f;
  Projectile* p = UseProjectile();
  p->start = source;
  p->end = target;
  p->end += v3f(proximity * cos(radian), proximity * sin(radian), 0.0f);
  p->wkind = kind;
  p->frame = 1;

  switch (kind) {
    case kWeaponBullet: {
      v3f distance = p->end - p->start;
      v3f dir = Normalize(distance);
      p->duration = distance.x / dir.x;
    } break;
    case kWeaponCount:
    case kWeaponMiningLaser:
    case kWeaponLaser: {
      p->duration = duration;
    } break;
  }
}

void
ProjectileCreate(const Unit* target, const Unit* source, float proximity,
                 WeaponKind kind)
{
  float radian = (float)(rand() % 360) * PI / 180.0f;
  Projectile* p = UseProjectile();
  p->start = source->transform.position;
  p->end = target->transform.position;
  p->target_id = target->id;
  p->end += v3f(proximity * cos(radian), proximity * sin(radian), 0.0f);
  p->wkind = kind;
  p->frame = 1;

  switch (kind) {
    case kWeaponBullet: {
      v3f distance = p->end - p->start;
      v3f dir = Normalize(distance);
      p->duration = distance.x / dir.x;
    } break;
    case kWeaponCount:
    case kWeaponLaser: {
      p->duration = kLaserDuration;
    } break;
  }
}

void
ProjectileSimulation()
{
  for (int i = 0; i < kUsedProjectile; ++i) {
    Projectile* p = &kProjectile[i];
    if (p->wkind != kWeaponLaser) continue;
    Unit* target = FindUnit(p->target_id);
    if (!target) continue;
    if (p->frame > kLaserDuration / 2) continue;
    target->health -= kLaserDamage;
  }

  for (int i = 0; i < kUsedProjectile; ++i) {
    Projectile* p = &kProjectile[i];
    if (p->wkind != kWeaponBullet) continue;
    Unit* target = FindUnit(p->target_id);
    if (!target) continue;

    v3f dir = Normalize(p->end - p->start);
    v3f position = p->start + dir * p->frame;
    float dsq = v3fDsq(position, target->transform.position);
    if (dsq < kHitDsq) {
      target->health -= kBulletDamage;
      *p = kZeroProjectile;
    }
  }

  for (int i = 0; i < kUsedProjectile; ++i) {
    Projectile* p = &kProjectile[i];
    unsigned frame = p->frame + (p->frame > 0);
    if (frame > p->duration) {
      *p = kZeroProjectile;
    } else {
      p->frame = frame;
    }
  }
}

}  // namespace simulation
