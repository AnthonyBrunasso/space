#pragma once

namespace mood {

Obstacle*
ObstacleCreate(v2f pos, v2f dims, ObstacleType type)
{
  Obstacle* obstacle = nullptr;
  switch (type) {
    case kObstacleBoost: {
      obstacle = UseEntityObstacle(pos, dims);
      obstacle->obstacle_type = type;
      physics::Particle2d* p = FindParticle(obstacle);
      SBIT(p->flags, physics::kParticleIgnoreGravity);
      SBIT(p->flags, physics::kParticleIgnoreCollisionResolution);
      SBIT(p->user_flags, kParticleBoost);
    } break;
    case kObstacleNone:
    default: {
      printf("%s Unknown obstacle type.", __FUNCTION__);
      return nullptr;
    } break;
  }
  return obstacle;
}

void
ObstacleUpdate()
{
  FOR_EACH_ENTITY_P(Obstacle, o, p, {
    switch (o->obstacle_type) {
      case kObstacleBoost: {
        if (kSim.frame % 2 == 0) {
          v2f start = math::RandomPointInRect(p->aabb());
          r32 max_height = p->aabb().Max().y - start.y;
          v2f end = start + v2f(0.f, math::Random(1.f, max_height));
          RenderCreatePixelLine(start, end, rgg::kLightBlue, 30);
        }
      } break;
      case kObstacleNone:
      default: {
        printf("%s Unknown obstacle type.", __FUNCTION__);
      } break;
    }
  });
}

}
