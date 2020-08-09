#pragma once

namespace mood {

ObstacleComponent*
ObstacleCreate(v2f pos, v2f dims, ObstacleType type)
{
  ObstacleComponent* obstacle = nullptr;
  switch (type) {
    case kObstacleBoost: {
      ecs::Entity* entity = ecs::UseEntity();
      obstacle = ecs::AssignObstacleComponent(entity);
      physics::Particle2d* p =
          physics::CreateParticle2d(pos, dims, entity->id);
      ecs::AssignPhysicsComponent(entity)->particle_id = p->id;
      obstacle->obstacle_type = type;
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
  ECS_ITR1(itr, kObstacleComponent);
  while (itr.Next()) {
    ObstacleComponent* o = itr.c.obstacle;
    physics::Particle2d* p = ecs::GetParticle(itr.e);
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
  }
}

}
