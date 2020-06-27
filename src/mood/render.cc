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
  physics::DebugRender(); 
  for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
    physics::Particle2d* p = &physics::kParticle2d[i];
    if (p == FindParticle(Player())) {
      rgg::RenderLineRectangle(p->aabb(), rgg::kGreen);
    } else {
      rgg::RenderLineRectangle(p->aabb(), rgg::kRed);
    }
  }
}

}
