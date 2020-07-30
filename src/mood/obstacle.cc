#pragma once

namespace mood {

Obstacle*
ObstacleCreate(v2f pos, ObstacleType type)
{
  Obstacle* obstacle = nullptr;
  switch (type) {
    case kObstacleBoost: {
      obstacle = UseEntityObstacle(pos, v2f(5.f, 5.f));
      obstacle->obstacle_type = type;
      physics::Particle2d* p = FindParticle(obstacle);
      SBIT(p->flags, physics::kParticleIgnoreGravity);
      SBIT(p->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(p->user_flags, kParticleBoost);
    } break;
    default: {
      printf("%s Unknown obstacle type.", __FUNCTION__);
      return nullptr;
    } break;
  }
  return obstacle;
}

}
