#pragma once

#include "entity.cc"
#include "util.cc"

namespace simulation
{
constexpr float kHitDsq = 25.f * 25.f;
constexpr float kLaserDamage = 0.25f;
constexpr float kBulletDamage = 0.50f;

void
ProjectileCreate(v3f at, float proximity, Unit* unit, WeaponKind kind)
{
  Projectile* proj = UseProjectile();
  proj->start = unit->transform.position;
  proj->end = at;
  float radian = (float)(rand() % 360) * PI / 180.0f;
  proj->end += v3f(proximity * cos(radian), proximity * sin(radian), 0.0f);
  // TODO(abrunasso): Idk man... How long should this go for...
  proj->wkind = kind;
  proj->duration = 100;
  proj->frame = 1;
}

void
ProjectileShootAt(Unit* target, float proximity, Unit* unit, WeaponKind kind)
{
  ProjectileCreate(target->transform.position, proximity, unit, kind);
  if (kind == kWeaponLaser) {
    target->health -= kLaserDamage;
  }
  unit->attack_frame = kResource[0].frame;
}

void
ProjectileSimulation()
{
  for (int i = 0; i < kUsedProjectile; ++i) {
    Projectile* p = &kProjectile[i];
    p->frame += (p->frame > 0);
    if (p->frame >= p->duration) {
      float dsq;
      uint64_t idx = v3fNearTransform(p->end, GAME_ITER(Unit, transform), &dsq);
      if (idx < kUsedUnit && dsq < kHitDsq) {
        kUnit[idx].health -= kBulletDamage;
      }
      *p = kZeroProjectile;
    }
  }
}

}  // namespace simulation
