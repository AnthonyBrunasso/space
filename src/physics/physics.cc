#pragma once

#include "common/common.cc"
#include "math/vec.h"
#include "math/rect.h"
#include "renderer/imui.cc"

namespace physics {

struct Particle2d {
  u32 id;
  u32 flags = 0;
  // NOTE: This should really not be modified outside of Integrate. If it is
  // do it knowing that collisions will only take place after position updates
  // therefore one could see particle penetration for a frame before resolution
  v2f position;
  v2f velocity;
  v2f acceleration;
  // Inverse mass of this particle. Inverse infinite mass can be reprented
  // with a value of 0 and since 0 mass objects have no physical
  // representation.
  r32 inverse_mass = 1.f;
  r32 damping = 0.05f; // Damping force applied to particle to slow it down.
  v2f force = {};      // Sum of all forces acting on this particle.
  v2f dims;            // Used to create the aabb.
  // Set to true when the particle is colliding with a 0.0 inverse mass
  // particle underneath it.
  b8 on_ground = false;
  b8 on_wall = false;

  // Particles keep forward and backward pointers so they can quickly sort
  // themselves after updating.
  u32 next_p2d_x = kInvalidId;
  u32 prev_p2d_x = kInvalidId;

  // Id to entity that contains this particle. Zero if not owned by an entity.
  u32 entity_id = 0;

  // If set to something other than UINT32_MAX will countdown to 0 and delete
  // itself.
  u32 ttl = UINT32_MAX;

  // Particles that have matching collision masks will not collide.
  u32 collision_mask = 0;

  Rectf
  aabb() const
  {
    return Rectf(position - dims / 2.f, dims);
  }

  r32
  mass() const
  {
    return inverse_mass > FLT_EPSILON ? 1.f / inverse_mass : FLT_MAX;
  }
};


struct Physics {
  // Acceleration of gravity.
  r32 gravity = 350.f;
  // Linked list in sorted order on x / y axis for collision checks.
  u32 p2d_head_x = kInvalidId;
  // If using DebugUI will render rectangles where collisions occur.
  b8 debug_render_collision = true;
};

static Physics kPhysics;

enum ParticleFlags {
  // Ignores the force of gravity if it's enabled.
  kParticleIgnoreGravity = 0,
  // If set the particle will not run its integration step.
  kParticleFreeze = 1,
  // If set the particle will be removed at the beginning of the next
  // integration step.
  kParticleRemove = 2,
  // If set on one of the colliding particles NEITHER particle will resolve
  // collision overlap.
  kParticleIgnoreCollisionResolution = 3,
  // If set velocity will not be dampened.
  kParticleIgnoreDamping = 4,
};

DECLARE_HASH_ARRAY(Particle2d, PHYSICS_PARTICLE_COUNT);

typedef void ApplyForceCallback(Particle2d* p);

#include "broadphase.cc"

Particle2d*
CreateParticle2d(v2f pos, v2f dims)
{
  Particle2d* particle = UseParticle2d();
  particle->position = pos;
  particle->dims = dims;
  BPInitP2d(particle);
  return particle;
}

Particle2d*
CreateInfinteMassParticle2d(v2f pos, v2f dims)
{
  Particle2d* particle = UseParticle2d();
  particle->position = pos;
  particle->dims = dims;
  particle->inverse_mass = 0.f;
  BPInitP2d(particle);
  return particle;
}

void
DeleteParticle2d(Particle2d* p)
{
  if (!p) return;
  SBIT(p->flags, physics::kParticleRemove);
}

void
DeleteParticle2d(u32 id)
{
  if (!id) return;
  DeleteParticle2d(FindParticle2d(id));
}

void
__ResolvePositionAndVelocity(
    Particle2d* p, v2f correction, const Rectf& intersection)
{
  if (p->inverse_mass < FLT_EPSILON) return;
  if (IsZero(p->velocity)) return;
  if (correction.x > 0.f) {
    if (p->aabb().x < intersection.x) {
      p->position.x -= correction.x;
    } else {
      p->position.x += correction.x;
    }
    // Don't zero out x velocity. This would cause the particle to stutter
    // when walking over multiple blocks.
    //p->velocity.x = 0.f;
  }
  if (correction.y > 0.f) {
    if (p->aabb().y < intersection.y) {
      p->position.y -= correction.y;
    } else {
      p->position.y += correction.y;
    }
    p->velocity.y = 0.f;
  }
}

void
__SetOnGround(Particle2d* p, const Rectf& intersection)
{
  if (!p->on_ground) {
    p->on_ground = intersection.y < p->aabb().y;
  }
}

void
__SetOnWall(Particle2d* p, const Rectf& intersection)
{
  if (!p->on_wall) {
    p->on_wall = intersection.x < p->aabb().x ||
                 intersection.Max().x > p->aabb().Max().x;
  }
}

void
Integrate(r32 dt_sec)
{

  assert(dt_sec > 0.f);
  // Delete any particles that must be deleted for this integration step.
  for (u32 i = 0; i < kUsedParticle2d;) {
    Particle2d* p = &kParticle2d[i];
    if (FLAGGED(p->flags, kParticleRemove) || p->ttl == 0) {
      BPDeleteP2d(p);
      continue;
    }
    if (p->ttl != UINT32_MAX) {
      --p->ttl;
    }
    ++i;
  }

  // Move all particles according to our laws of physics ignoring collisions.
  for (u32 i = 0; i < kUsedParticle2d; ++i) {
    Particle2d* p = &kParticle2d[i];
    if (FLAGGED(p->flags, kParticleFreeze)) continue;
    // Infinite mass object - do nothing.
    if (p->inverse_mass <= 0.f) continue;
    
    // Set after collision code runs.
    p->on_ground = false;
    p->on_wall = false;
    // F = m * a
    // a = F * (1 / m)
    v2f acc = p->acceleration;
    p->acceleration += p->force * p->inverse_mass;
    if (!FLAGGED(p->flags, kParticleIgnoreGravity)) {
      p->acceleration -= v2f(0.f, kPhysics.gravity);
    }
    p->velocity += p->acceleration * dt_sec;
    p->position += p->velocity * dt_sec;
    if (!FLAGGED(p->flags, kParticleIgnoreDamping)) {
      p->velocity *= pow(p->damping, dt_sec);
    }
    // Acceleration applied from forces only last a single frame.
    // Reverting here allows any user imposed acceleration to stick around.
    p->acceleration = acc;
    // Force applied over a single integration step then reset.
    p->force = {};

    BPUpdateP2d(p);
  }

  BPCalculateCollisions();

  // Resolve collisions.
  for (u32 i = 0; i < kUsedBP2dCollision; ++i) {
    BP2dCollision* c = &kBP2dCollision[i];

    // If either particle completely ignore collision continue.
    if (FLAGGED(c->p1->flags, kParticleIgnoreCollisionResolution)) continue;
    if (FLAGGED(c->p2->flags, kParticleIgnoreCollisionResolution)) continue;

    // If the particles have matching collision masks continue.
    if (c->p1->collision_mask && c->p2->collision_mask &&
        c->p1->collision_mask == c->p2->collision_mask) continue;

    // Another correction may have moved this collision out of intersection.
    if (!math::IntersectRect(c->p1->aabb(), c->p2->aabb(), &c->intersection)) {
      continue;
    }

    // Use min axis of intersection to correct collisions.
    v2f correction;
    if (c->intersection.width < c->intersection.height) {
      correction = v2f(c->intersection.width, 0.f);
    } else {
      correction = v2f(0.f, c->intersection.height);
    }

    __ResolvePositionAndVelocity(c->p1, correction, c->intersection);
    __ResolvePositionAndVelocity(c->p2, correction, c->intersection);

    __SetOnGround(c->p1, c->intersection);
    __SetOnGround(c->p2, c->intersection);

    __SetOnWall(c->p1, c->intersection);
    __SetOnWall(c->p2, c->intersection);

    BPUpdateP2d(c->p1);
    BPUpdateP2d(c->p2);
  }
}

void
DebugUI(v2f screen)
{
  static const u32 kUIBufferSize = 64;
  static char kUIBuffer[kUIBufferSize];
  static b8 enable_physics = true;
  static v2f physics_pos(0.f, screen.y - 300.f);
  imui::PaneOptions options;
  options.width = options.max_width = 365.f;
  options.max_height = 500.f;
  imui::Begin("Physics", imui::kEveryoneTag, options, &physics_pos,
              &enable_physics);
  static const r32 kWidth = 130.f;
  imui::SameLine();
  imui::Width(80);
  imui::Text("Collision");
  imui::Checkbox(16.f, 16.f, &kPhysics.debug_render_collision);
  imui::NewLine();
  imui::SameLine();
  imui::Width(80);
  imui::Text("X Head");
  snprintf(kUIBuffer, kUIBufferSize, "%u", kPhysics.p2d_head_x);
  imui::Text(kUIBuffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(80);
  imui::Text("Gravity");
  snprintf(kUIBuffer, kUIBufferSize, "%.2f", kPhysics.gravity);
  imui::Width(100.f);
  imui::Text(kUIBuffer);
  if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
    kPhysics.gravity -= 10.f;
  }
  imui::Space(imui::kHorizontal, 5.f);
  if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
    kPhysics.gravity += 10.f;
  }
  imui::NewLine();
  for (u32 i = 0; i < kUsedParticle2d; ++i) {
    Particle2d* p = &kParticle2d[i];
    imui::SameLine();
    imui::Width(80);
    imui::TextOptions o;
    o.highlight_color = rgg::kRed;
    if (imui::Text("Particle", o).highlighted) {
      rgg::DebugPushRect(p->aabb(), rgg::kGreen);
      if (p->next_p2d_x) {
        rgg::DebugPushRect(
            FindParticle2d(p->next_p2d_x)->aabb(), rgg::kBlue);
      }
      if (p->prev_p2d_x) {
        rgg::DebugPushRect(
            FindParticle2d(p->prev_p2d_x)->aabb(), rgg::kPurple);
      }
    }
    snprintf(kUIBuffer, kUIBufferSize, "%u", p->id);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::Indent(2);
    if (imui::ButtonCircle(8.f, v4f(1.f, 0.f, 0.f, .7f)).clicked) {
      DeleteParticle2d(p);
    }
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Next");
    snprintf(kUIBuffer, kUIBufferSize, "%u", p->next_p2d_x);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Prev");
    snprintf(kUIBuffer, kUIBufferSize, "%u", p->prev_p2d_x);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("On Ground");
    snprintf(kUIBuffer, kUIBufferSize, "%i", p->on_ground);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("On Wall");
    snprintf(kUIBuffer, kUIBufferSize, "%i", p->on_wall);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Position");
    snprintf(kUIBuffer, kUIBufferSize, "%.3f,%.3f", p->position.x,
             p->position.y);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Velocity");
    snprintf(kUIBuffer, kUIBufferSize, "%.3f,%.3f", p->velocity.x,
             p->velocity.y);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Acceleration");
    snprintf(kUIBuffer, kUIBufferSize, "%.3f,%.3f", p->acceleration.x,
             p->acceleration.y);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Inverse Mass");
    snprintf(kUIBuffer, kUIBufferSize, "%.3f", p->inverse_mass);
    imui::Width(kWidth / 2.f);
    imui::Text(kUIBuffer);
    if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
      p->inverse_mass -= .1f;
    }
    imui::Space(imui::kHorizontal, 5.f);
    if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
      p->inverse_mass += .1f;
    }
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Damping");
    snprintf(kUIBuffer, kUIBufferSize, "%.3f", p->damping);
    imui::Width(kWidth / 2.f);
    imui::Text(kUIBuffer);
    if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
      p->damping -= .01f;
    }
    imui::Space(imui::kHorizontal, 5.f);
    if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
      p->damping += .01f;
    }
    p->damping = CLAMPF(p->damping, 0.f, 1.0f);
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Width");
    snprintf(kUIBuffer, kUIBufferSize, "%.3f", p->dims.x);
    imui::Width(kWidth / 2.f);
    imui::Text(kUIBuffer);
    if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
      p->dims.x -= 1.f;
    }
    imui::Space(imui::kHorizontal, 5.f);
    if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
      p->dims.x += 1.f;
    }
    if (imui::Button(16.f, 16.f, rgg::kGreen).clicked) {
      p->dims.x -= 10.f;
    }
    imui::Space(imui::kHorizontal, 5.f);
    if (imui::Button(16.f, 16.f, rgg::kGreen).clicked) {
      p->dims.x += 10.f;
    }
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Height");
    snprintf(kUIBuffer, kUIBufferSize, "%.3f", p->dims.y);
    imui::Width(kWidth / 2.f);
    imui::Text(kUIBuffer);
    if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
      p->dims.y -= 1.f;
    }
    imui::Space(imui::kHorizontal, 5.f);
    if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
      p->dims.y += 1.f;
    }
    if (imui::Button(16.f, 16.f, rgg::kGreen).clicked) {
      p->dims.y -= 10.f;
    }
    imui::Space(imui::kHorizontal, 5.f);
    if (imui::Button(16.f, 16.f, rgg::kGreen).clicked) {
      p->dims.y += 10.f;
    }
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Freeze");
    b8 set = FLAGGED(p->flags, kParticleFreeze);
    imui::Checkbox(16, 16, &set);
    if (set) {
      SBIT(p->flags, kParticleFreeze);
    } else {
      CBIT(p->flags, kParticleFreeze);
    }
    imui::NewLine();
    imui::SameLine();
    imui::Width(kWidth);
    imui::Text("Ignore Gravity");
    set = FLAGGED(p->flags, kParticleIgnoreGravity);
    imui::Checkbox(16, 16, &set);
    if (set) {
      SBIT(p->flags, kParticleIgnoreGravity);
    } else {
      CBIT(p->flags, kParticleIgnoreGravity);
    }
    imui::NewLine();
    imui::Indent(0);
    BPUpdateP2d(p);
  }
  imui::End();
}

void
DebugRender()
{
  if (kPhysics.debug_render_collision) {
    for (u32 i = 0; i < kUsedBP2dCollision; ++i) {
      BP2dCollision* c = &kBP2dCollision[i];
      rgg::RenderLineRectangle(c->intersection, rgg::kWhite);
    }
  }
}

}  // namesapce physics
