#pragma once

#include "animation/sprite.cc"
#include "renderer/texture.cc"
#include "mood/sim.cc"

namespace mood {

// defined in interaction.cc
b8 IsInEditMode(u32* type);
void GetTileEditInfo(v2f* pos, u32* texture_id, Rectf* texture_subrect);

struct Effect {
  Rectf rect;
  v4f color = { 0.f, 0.f, 0.f, 1.f };
  u32 ttl = 0;
  r32 rotate = 0.f;
  r32 rotate_delta = 0.f;
};

struct Texture {
  u32 texture_id;
  Rectf rect;
  Rectf subrect;
  SPRITE_LABEL(label_name);
};

DECLARE_ARRAY(Effect, 64);
DECLARE_ARRAY(Texture, 256);

struct Render {
  u32 snail_id;
  u32 character_id;
};

static Render kRender;

static b8 kRenderAabb = false;
static b8 kRenderSpawner = false;
static b8 kRenderGrid = false;

#define PIXEL_ART_OBSERVER()                                       \
  rgg::Camera* c = rgg::CameraGetCurrent();                        \
  rgg::ModifyObserver mod(                                         \
      math::Ortho(mood::kRenderTargetWidth, 0.f,                   \
                  mood::kRenderTargetHeight, 0.f, -1.f, 1.f),      \
      math::LookAt(c->position, c->position + v3f(0.f, 0.f, -1.f), \
                   v3f(0.f, -1.f, 0.f)));

void
RenderInitialize()
{
  kUsedTexture = 0;
  kUsedEffect = 0;

  rgg::TextureInfo info;
  info.min_filter = GL_NEAREST;
  info.mag_filter = GL_NEAREST;
  kRender.snail_id =
      rgg::LoadTextureAndSprite("asset/snail.tga", "asset/snail.anim", info);
  assert(kRender.snail_id);
  kRender.character_id  =
      rgg::LoadTextureAndSprite("asset/main_character.tga", "asset/main_character.anim",
                                info);
  assert(kRender.character_id);
}

void
RenderUpdate()
{
  for (u32 i = 0; i < kUsedEffect;) {
    --kEffect[i].ttl;
    kEffect[i].rotate += kEffect[i].rotate_delta;
    if (!kEffect[i].ttl) {
      EraseEffect(i);
      continue;
    }
    ++i;
  }
}

void
RenderCreateEffect(Rectf rect, v4f color, u32 ttl, r32 rotate_delta)
{
  Effect* e = UseEffect();
  e->rect = rect;
  e->color = color;
  e->ttl = ttl;
  e->rotate_delta = rotate_delta;
}

void
RenderCreateTexture(u32 id, Rectf rect, Rectf subrect, char* label_name)
{
  Texture* t = UseTexture();
  t->texture_id = id;
  t->rect = rect;
  t->subrect = subrect;
  strcpy(t->label_name, label_name);
}

void
RenderDeleteTexture(v2f pos)
{
  for (u32 i = 0; i < kUsedTexture;) {
    if (math::PointInRect(pos, kTexture[i].rect)) {
      CompressTexture(i);
      break;
    }
    ++i;
  }
}

void
Render()
{
  //v2f clickpos = rgg::CameraRayFromMouseToWorld(
  //    window::GetCursorPosition(), 0.f).xy();
  //rgg::DebugPushPoint(clickpos, 2.f, rgg::kRed);
  //v2i tile = WorldToTile(clickpos);
  //printf("(%.2f %.2f) to (%i %i)\n", clickpos.x, clickpos.y, tile.x, tile.y);
  // 66 -> 64
  // 66 - 66 % 32
  //s32 x = (s32)((s32)clickpos.x / (s32)kTileWidth) * (s32)kTileWidth;
  //s32 y = (s32)((s32)clickpos.y / (s32)kTileHeight) * (s32)kTileHeight;
  //rgg::DebugPushRect(Rectf(v2f(x, y), v2f(16.f, 16.f)), rgg::kRed);
  //rgg::DebugPushRect(
  //    Rectf(v2f(clickpos.x - ((s32)clickpos.x % (s32)kTileWidth),
  //              clickpos.y - ((s32)clickpos.y % (s32)kTileHeight)),
  //          kTileWidth, kTileHeight), rgg::kRed);
  for (u32 i = 0; i < kUsedEffect; ++i) {
    Effect* e = &kEffect[i];
    rgg::RenderLineRectangle(e->rect, 0.f, e->rotate, e->color);
  }
  rgg::DebugRenderWorldPrimitives();

  for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
    physics::Particle2d* p = &physics::kParticle2d[i];
    Entity* e = FindEntity(p->entity_id);
    if (p->user_flags) {
      if (FLAGGED(p->user_flags, kParticleBlood)) {
        rgg::RenderRectangle(p->aabb(), rgg::kRed);
      } else if (FLAGGED(p->user_flags, kParticleSpark)) {
        rgg::RenderRectangle(p->aabb(), v4f(.9f, .88f, .1f, 1.f));
      } else if (FLAGGED(p->user_flags, kParticleCollider)) {
        if (kRenderAabb) rgg::RenderLineRectangle(p->aabb(), rgg::kRed);
      } else if (FLAGGED(p->user_flags, kParticleSpawner)) {
        if (kRenderSpawner) rgg::RenderLineRectangle(p->aabb(), rgg::kPurple);
      }
    } else if (e && e->type == kEntityTypeProjectile) {
        rgg::RenderRectangle(p->Rect(), 0.f, p->rotation, rgg::kWhite);
    }
  }
  animation::Sprite* character_sprite = rgg::GetSprite(kRender.character_id);
  animation::Sprite* snail_sprite = rgg::GetSprite(kRender.snail_id);
  //physics::DebugRender(); 
  FOR_EACH_ENTITY_P(Character, c, p, {
    if (c == Player()) {
      Rectf paabb = p->aabb();
      bool mirror = c->facing.x >= 0.f ? false : true;
      if (fabs(p->velocity.x) > 10.f) {
        animation::SetLabel("walk", character_sprite);
      } else animation::SetLabel("idle", character_sprite);
      v2f end = p->position + c->aim_dir * 100.f;
      rgg::RenderLine(p->position, end, v4f(1.f, 0.f, 0.f, 0.25f));
      rgg::RenderTexture(
            kRender.character_id,
            animation::Update(character_sprite, &c->anim_frame),
            Rectf(paabb.x - 12.f, paabb.y,
                  character_sprite->width,
                  character_sprite->height), mirror);
      if (kRenderAabb) rgg::RenderLineRectangle(paabb, rgg::kRed);
      continue;
    }
    Rectf aabb = p->aabb();
    r32 r_width = aabb.width * kEnemyHealthWidthPercent;
    Rectf pb_rect(aabb.x, aabb.Max().y + 1.f,
                  r_width, kEnemyHealthHeight);
    rgg::RenderProgressBar(pb_rect, 0.f, c->health, c->max_health,
                           v4f(1.f, 0.f, 0.f, .7f), v4f(.8f, .8f, .8f, .5f));
    const u32* behavior = nullptr;
    if (BB_GET(c->bb, kAIBbType, behavior)) {
      switch (*behavior) {
        case kBehaviorSimple: {
          Rectf paabb = p->aabb();
          // TODO: How should I handle this?
          bool mirror = p->velocity.x >= 0.f ? true : false;
          rgg::RenderTexture(
              kRender.snail_id,
              animation::Update(snail_sprite, &c->anim_frame),
              Rectf(paabb.x - 8.f, paabb.y - 17.f,
                    snail_sprite->width,
                    snail_sprite->height), mirror);
          if (kRenderAabb) rgg::RenderLineRectangle(paabb, rgg::kRed);
        } break;
        case kBehaviorSimpleFlying: {
          rgg::RenderCircle(
              p->position, p->aabb().width / 2.f, v4f(1.f, 0.f, 0.f, 1.f));
        } break;
      }
    }
  });

  for (u32 i = 0; i < kUsedTexture; ++i) {
    Texture* t = &kTexture[i];
    rgg::RenderTexture(t->texture_id, t->subrect, t->rect);
  }

  if (kRenderGrid) {
    v4f colors[] = {
        v4f(0.207f, 0.317f, 0.360f, 0.60f),
        v4f(0.050f, 0.215f, 0.050f, 0.45f),
    };
    r32 total_width = kTileWidth * 1000.f;
    r32 total_height = kTileHeight * 1000.f;
    rgg::RenderGrid(
        v2f(kTileWidth, kTileHeight),
        Rectf(-total_width / 2.f, -total_height / 2.f,
              total_width, total_height),
        ARRAY_LENGTH(colors), colors);
  }

  u32 type;
  if (IsInEditMode(&type)) {
    v2f pos;
    u32 texture_id;
    Rectf texture_subrect;
    GetTileEditInfo(&pos, &texture_id, &texture_subrect);
    switch (type) {
      case 3:
      case 1: {
        rgg::RenderTexture(
            texture_id, texture_subrect,
            Rectf(pos, v2f(texture_subrect.width, texture_subrect.height)));
      } break;
      case 2: {
        rgg::RenderLineRectangle(
            Rectf(pos, v2f(texture_subrect.width, texture_subrect.height)),
            rgg::kRed);
      } break;
    }
  }
  
  // Render the players health...
  auto dims = window::GetWindowSize();
  rgg::ModifyObserver mod(
      math::Ortho2(kRenderTargetWidth, 0.0f, kRenderTargetHeight,
                   0.0f, 0.0f, 0.0f), math::Identity());
  Character* c = Player();
  if (c) {
    rgg::RenderProgressBar(
        Rectf(kRenderTargetWidth / 2.f - kPlayerHealthBarWidth / 2.f,
              kRenderTargetHeight - 20.f, kPlayerHealthBarWidth,
              kPlayerHealthBarHeight),
        0.f, c->health, c->max_health, rgg::kRed, rgg::kWhite);
  }
}

}
