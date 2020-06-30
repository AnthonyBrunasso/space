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
  FOR_EACH_ENTITY_P(Character, c, p, {
    if (c == Player()) {
      Rectf paabb = p->aabb();
      rgg::RenderRectangle(paabb, rgg::kGreen);
      if (FLAGGED(c->character_flags, kCharacterAim)) {
        v2f start = v2f(paabb.Center().x, paabb.Max().y);
        v2f end = start + c->aim_dir * 100.f;
        rgg::RenderLine(start, end, v4f(1.f, 0.f, 0.f, 0.5f));
      }
      continue;
    }
    Rectf aabb = p->aabb();
    Rectf pb_rect(aabb.x, aabb.Max().y + .5f, aabb.width, 1.f);
    rgg::RenderProgressBar(pb_rect, 0.f, c->health, c->max_health,
                           v4f(1.f, 0.f, 0.f, .7f), v4f(.8f, .8f, .8f, .5f));
    const u32* behavior = nullptr;
    if (BB_GET(c->bb, kAIBbType, behavior)) {
      switch (*behavior) {
        case kBehaviorSimple: {
          rgg::RenderRectangle(p->aabb(), v4f(1.f, 0.f, 0.f, .8f));
        } break;
        case kBehaviorSimpleFlying: {
          rgg::RenderCircle(
              p->position, p->aabb().width / 2.f, v4f(1.f, 0.f, 0.f, .7f));
        } break;
      }
    }
  });

  for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
    physics::Particle2d* p = &physics::kParticle2d[i];
    Entity* e = FindEntity(p->entity_id);
    if (p->user_flags) {
      if (FLAGGED(p->user_flags, kParticleBlood)) {
        rgg::RenderRectangle(p->aabb(), rgg::kRed);
      } else if (FLAGGED(p->user_flags, kParticleSpark)) {
        rgg::RenderRectangle(p->aabb(), v4f(.9f, .88f, .1f, 1.f));
      } else if (FLAGGED(p->user_flags, kParticleCollider)) {
        rgg::RenderRectangle(p->aabb(), v4f(.5f, .5f, .5f, 1.f));
      } else if (FLAGGED(p->user_flags, kParticleTest)) {
        rgg::RenderLineRectangle(
            p->Rect(), 0.f, 0.f, v4f(1.f, 0.5f, 0.2f, 1.f));
      }
    } else if (e && e->type == kEntityTypeProjectile) {
      rgg::RenderRectangle(p->aabb(), rgg::kWhite);
    }
  }
  
  // Render the players health...
  auto dims = window::GetWindowSize();
  rgg::ModifyObserver mod(math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f),
                          math::Identity());
  Character* c = Player();
  rgg::RenderProgressBar(
      Rectf(dims.x / 2.f - kPlayerHealthBarWidth / 2.f,
            50.f, kPlayerHealthBarWidth, kPlayerHealthBarHeight),
      0.f, c->health, c->max_health, rgg::kRed, rgg::kWhite);
}

}
