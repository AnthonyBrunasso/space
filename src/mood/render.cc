#pragma once

#include "mood/sim.cc"

namespace mood {

struct Effect {
  Rectf rect;
  v4f color = { 0.f, 0.f, 0.f, 1.f };
  u32 ttl = 0;
};

DECLARE_ARRAY(Effect, 64);

void
RenderUpdate()
{
  for (u32 i = 0; i < kUsedEffect;) {
    --kEffect[i].ttl;
    if (!kEffect[i].ttl) {
      EraseEffect(i);
      continue;
    }
    ++i;
  }
}

void
RenderCreateEffect(Rectf rect, v4f color, u32 ttl)
{
  Effect* e = UseEffect();
  e->rect = rect;
  e->color = color;
  e->ttl = ttl;
}

void
Render()
{
  for (u32 i = 0; i < kUsedEffect; ++i) {
    Effect* e = &kEffect[i];
    rgg::RenderLineRectangle(e->rect, e->color);
  }
  rgg::DebugRenderPrimitives();
  //physics::DebugRender(); 

  for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
    physics::Particle2d* p = &physics::kParticle2d[i];
    Entity* e = FindEntity(p->entity_id);
    if (p == FindParticle(Player())) {
      rgg::RenderLineRectangle(p->aabb(), rgg::kGreen);
    } else {
      if (p->user_flags) {
        if (FLAGGED(p->user_flags, kParticleBlood)) {
          rgg::RenderRectangle(p->aabb(), rgg::kRed);
        } else if (FLAGGED(p->user_flags, kParticleSpark)) {
          rgg::RenderRectangle(p->aabb(), v4f(.9f, .88f, .1f, 1.f));
        } else if (FLAGGED(p->user_flags, kParticleCollider)) {
          rgg::RenderRectangle(p->aabb(), v4f(.5f, .5f, .5f, 1.f));
        }
      } else if (e && e->type == kEntityTypeProjectile) {
        rgg::RenderRectangle(p->aabb(), rgg::kWhite);
      } else if (e && e->type == kEntityTypeCharacter) {
        rgg::RenderRectangle(p->aabb(), v4f(1.f, 0.f, 0.f, .8f));
      } else {
        rgg::RenderLineRectangle(p->aabb(), v4f(.5f, 0.f, .75f, 1.f));
      }
    }
  }

  FOR_EACH_ENTITY_P(Character, c, p, {
    if (c == Player()) continue;
    Rectf aabb = p->aabb();
    Rectf pb_rect(aabb.x, aabb.Max().y + .5f, aabb.width, 1.f);
    rgg::RenderProgressBar(pb_rect, 0.f, c->health, c->max_health, rgg::kRed,
                           rgg::kWhite);
  });
}

}
