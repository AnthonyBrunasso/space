#pragma once

namespace simulation
{

void
ProjectileShootLaserAt(v3f at, float proximity, Unit* unit)
{
  Projectile* proj = UseProjectile();
  proj->dir = math::Normalize(at - unit->transform.position);
  proj->start = unit->transform.position + proj->dir * 10.f;
  proj->end = at;
  float radian = (float)(rand() % 360) * PI / 180.0f;
  proj->end += v3f(proximity * cos(radian), proximity * sin(radian), 0.0f);
  // TODO(abrunasso): Idk man... How long should this go for...
  proj->duration = 100;
  unit->attack_frame = kResource[0].frame;
}

void
ProjectileSimulation()
{
  for (int i = 0; i < kUsedProjectile;) {
    Projectile* p = &kProjectile[i];
    // p->transform.position = p->transform.position + p->dir * p->speed;
    --p->duration;
    if (p->duration <= 0) {
      CompressProjectile(i);
      continue;
    }
    ++i;
  }
}

}
