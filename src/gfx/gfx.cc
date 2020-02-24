#pragma once

#include "imui.cc"
#include "math/vec.h"
#include "platform/platform.cc"
#include "renderer/renderer.cc"

#include "../simulation/simulation.cc"

namespace gfx
{
struct Gfx {
  RenderTag asteroid_tag;
  RenderTag pod_tag;
  RenderTag missile_tag;
  RenderTag cryo_tag;
  RenderTag exhaust_tag;
};

static Gfx kGfx;
static v4f kWhite = v4f(1.f, 1.f, 1.f, 1.f);
static v4f kRed = v4f(1.f, 0.f, 0.f, 1.f);
static const math::Quatf kDefaultRotation = math::Quatf(0.f, 0.f, 0.f, 1.f);
static v3f kDefaultScale = v3f(1.f, 1.f, 1.f);
static v3f kTileScale = v3f(0.5f, 0.5f, 1.f);

bool
Initialize()
{
  int window_result = window::Create("Space", 1280, 720);
  printf("Window create result: %i\n", window_result);
  auto status = rgg::Initialize();
  constexpr int kVertCount = 29;
  constexpr int kFloatCount = kVertCount * 3;
  // clang-format off
  GLfloat asteroid[kFloatCount] = {
      0.f,   1.6f,   0.f,   0.2f,   1.5f,   0.f,   0.4f,  1.6f,  0.f,   0.6f,
      1.6f,  0.f,    0.68f, 1.9f,   0.f,    1.1f,  1.8f,  0.f,   1.6f,  1.7f,
      0.f,   1.8f,   0.9f,  0.f,    2.3f,   0.3f,  0.f,   2.4f,  -0.5f, 0.f,
      2.f,   -0.8f,  0.f,   1.5f,   -1.1f,  0.f,   0.7f,  -1.f,  0.f,   0.5f,
      -1.1f, 0.f,    0.2f,  -1.3f,  0.f,    -0.3f, -1.4f, 0.f,   -1.1f, -1.1f,
      0.f,   -1.3f,  -0.6f, 0.f,    -1.25f, -0.2f, 0.f,   -1.5f, 0.5f,  0.f,
      -1.4f, 0.4f,   0.f,   -1.65f, 1.f,    0.f,   -1.6f, 1.3f,  0.f,   -1.6f,
      1.7f,  0.f,    -1.4f, 1.9f,   0.f,    -1.f,  2.05f, 0.f,   -0.7f, 2.07f,
      0.f,   -0.65f, 2.2f,  0.f,    -0.5f,  2.25f, 0.f};
  constexpr int kPodVert = 6;
  GLfloat pod[kPodVert*3] = 
  {0.0f, 0.0f, 0.0f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
   1.5f, 1.0f, 0.0f, 0.5f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f};
  constexpr int kMissileVert = 4;
  GLfloat missile[kPodVert*3] = 
  {0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f,
   2.0f, 4.0f, 0.0f, 0.0f, 4.0f, 0.0f};
  constexpr int kCryoVert = 6;
  GLfloat cryo[kCryoVert*3] = 
  {-1.0f, 0.0f, 0.0f, 0.0f, -2.0f, 0.0f,
   1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
   0.0f, 3.0f, 0.f, -1.0f, 1.0f, 0.0f };
  constexpr int kExhaustVert = 12;
  GLfloat exhaust[kExhaustVert*3] = 
  {0.0f, 3.0f, 0.0f, 3.0f, 3.0f, 0.0f,
   4.0f, 2.7f, 0.0f, 3.5f, 2.4f, 0.0f,
   4.2f, 2.1f, 0.0f, 3.5f, 1.8f, 0.0f,
   4.2f, 1.5f, 0.0f, 3.5f, 1.2f, 0.0f,
   4.2f, 0.9f, 0.0, 3.5f, 0.6f, 0.0f,
   3.0, 0.3f, 0.0, 0.0f, 0.0f, 0.0f};
  // clang-format on
  for (int i = 0; i < kFloatCount; ++i) asteroid[i] *= 15.f;      // HA
  for (int i = 0; i < kPodVert * 3; ++i) pod[i] *= 22.f;          // HA
  for (int i = 0; i < kMissileVert * 3; ++i) missile[i] *= 15.f;  // HA
  for (int i = 0; i < kCryoVert * 3; ++i) cryo[i] *= 12.f;        // HA
  for (int i = 0; i < kExhaustVert * 3; ++i) exhaust[i] *= 15.f;  // HA
  kGfx.asteroid_tag = rgg::CreateRenderable(kVertCount, asteroid, GL_LINE_LOOP);
  kGfx.pod_tag = rgg::CreateRenderable(kPodVert, pod, GL_LINE_LOOP);
  kGfx.missile_tag = rgg::CreateRenderable(kMissileVert, missile, GL_LINE_LOOP);
  kGfx.cryo_tag = rgg::CreateRenderable(kCryoVert, cryo, GL_LINE_LOOP);
  kGfx.exhaust_tag = rgg::CreateRenderable(kExhaustVert, exhaust, GL_LINE_LOOP);
  return status;
}

void
AlignToGrid(v2f grid, math::Rectf* world)
{
  float x_align = fmodf(world->min.x, grid.x);
  float y_align = fmodf(world->min.y, grid.y);

  world->min.x -= x_align + grid.x;
  world->min.y -= y_align + grid.y;
}

void
RenderCrew()
{
  using namespace simulation;

  // Crew rendering
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (unit->inspace) continue;

    v4f color;
    switch (unit->kind) {
      case kOperator:
        color = v4f(0.50f, .33f, .33f, 1.f);
        break;
      case kMilitary:
        color = v4f(0.80, 0.10f, 0.10f, 1.f);
        break;
      default:
        continue;
    }

    if (IsUnitSelected(unit->id)) {
      color = v4f(0.26f, 0.33f, 0.68f, 1.f);
    }

    rgg::RenderRectangle(unit->transform.position, unit->transform.scale,
                         kDefaultRotation, color);

    if (unit->spacesuit) {
      rgg::RenderCircle(unit->transform.position, 12.f, 14.f,
                        v4f(0.99f, 0.33f, 0.33f, 1.f));
    }
  }

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (unit->uaction != kUaMove) continue;

    // Show the path they are on if they have one.
    v2i start = WorldToTilePos(unit->transform.position);
    v3f* dest = nullptr;
    if (!BB_GET(unit->bb, kUnitDestination, dest)) continue;
    v2i end = WorldToTilePos(*dest);

    auto* path = PathTo(start, end);
    if (!path || path->size <= 1) {
      continue;
    }

    for (int i = 0; i < path->size; ++i) {
      auto* t = &path->tile[i];
      rgg::RenderRectangle(v3f(TilePosToWorld(*t)),
                           v3f(1.f / 3.f, 1.f / 3.f, 1.f), kDefaultRotation,
                           v4f(0.33f, 0.33f, 0.33f, 0.40f));
    }
  }
}

v3f
ModuleColor(unsigned mkind)
{
  switch (mkind) {
    case kModPower:
      return v3f(0.0f, 0.0f, 0.75f);
    case kModEngine:
      return v3f(1.0f, 0.0f, 1.f);
    case kModMine:
      return v3f(0.0, 0.75f, 0.0f);
    case kModTurret:
      return v3f(1.f, 0.f, 0.f);
  }

  return v3f();
}

void
Render(v2f screen)
{
  using namespace simulation;

  for (int i = 0; i < kUsedGrid; ++i) {
    const v2f grid_dims(kTileWidth, kTileHeight);
    v4f colors[] = {
        v4f(0.207f, 0.317f, 0.360f, 0.60f),
        v4f(0.050f, 0.215f, 0.050f, 0.45f),
    };
    math::Rectf bounds = kGrid[i].bounds;
    bounds.min += kGrid[i].transform.position.xy() -
                   v2f(kTileWidth / 2.f, kTileHeight / 2.f);
    bounds.max += kGrid[i].transform.position.xy()  -
                   v2f(kTileWidth / 2.f, kTileHeight / 2.f);
    rgg::RenderGrid(grid_dims, bounds, ARRAY_LENGTH(colors), colors);
  }

  // Modules are always visible
  constexpr float min_visibility = .2f;

  for (int i = 0; i < kUsedModule; ++i) {
    Module* module = &kModule[i];
    v3f mcolor = ModuleColor(module->mkind);
    mcolor *= CLAMPF(min_visibility, kShip[0].sys[module->mkind], 1.0f);
    v4f color(mcolor.x, mcolor.y, mcolor.z, 1.f);
    ;
    rgg::RenderRectangle(
        v3f(simulation::TilePosToWorld(v2i{(int)module->cx, (int)module->cy})),
        v3f(1.f / 2.f, 1.f / 2.f, 1.f), kDefaultRotation, color);
  }

  static float escale = .1f;
  if (escale > 2.f) {
    escale = .1f;
  }

  {
    v3f world;
    for (int i = 0; i < kUsedModule; ++i) {
      Module* mod = &kModule[i];
      if (mod->mkind != kModEngine) continue;
      float engine_power =
          fminf(kShip[0].sys[kModEngine], kShip[0].sys[kModPower]);
      float visibility = fmaxf(min_visibility, engine_power);
      v3f mcolor = ModuleColor(mod->mkind);
      mcolor *= visibility;
      v2i hack = {-1, 2};
      world = TilePosToWorld(v2i(mod->cx, mod->cy) + hack);
      v4f color(mcolor.x, mcolor.y, mcolor.z, 1.f);
      rgg::RenderTag(kGfx.exhaust_tag, world, v3f(escale, 1.f, 1.f),
                     kDefaultRotation, color);
      rgg::RenderTag(kGfx.exhaust_tag, world,
                     v3f(fmodf(escale + 1.f, 2.f), 1.f, 1.f), kDefaultRotation,
                     color);
      escale += .1f;
      break;
    }
  }

  for (int i = 0; i < kUsedConsumable; ++i) {
    Consumable* c = &kConsumable[i];
    const Tile* tile = TilePtr(v2i(c->cx, c->cy));
    if (!tile || !tile->explored) continue;

    v4f color = v4f(1.f, 1.f, 1.f, 1.f);
    v3f world = TileToWorld(*tile);
    if (c->cryo_chamber) {
      rgg::RenderTag(kGfx.cryo_tag, world, kTileScale, kDefaultRotation, color);
    } else {
      rgg::RenderTriangle(world, kTileScale, kDefaultRotation, color);
    }
  }

  float fft = kShip[0].ftl_frame * (1.f / kFtlFrameTime);
  float ship_alpha = 1.f - fft;
  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      const Tile* tile = TilePtr(v2i(j, i));

      v4f color;
      if (!tile->explored) {
        color = v4f(0.3f, 0.3f, 0.3f, .7);
        rgg::RenderRectangle(v3f(TileToWorld(*tile)), kTileScale,
                             kDefaultRotation, color);
      } else if (tile->blocked) {
        color = v4f(1.f, 1.f, 1.f, ship_alpha);
        rgg::RenderRectangle(v3f(TileToWorld(*tile)), kTileScale,
                             kDefaultRotation, color);
      } else if (tile->nooxygen) {
        color = v4f(1.f, 0.f, .2f, .4);
        rgg::RenderRectangle(v3f(TileToWorld(*tile)), kTileScale,
                             kDefaultRotation, color);
      } else if (tile->shroud) {
        color = v4f(0.3f, 0.3f, 0.3f, .4);
        rgg::RenderRectangle(v3f(TileToWorld(*tile)), kTileScale,
                             kDefaultRotation, color);
      }
    }
  }

  for (int i = 0; i < kUsedPlayer; ++i) {
    Player* p = &kPlayer[i];
    // Hover selection
    v2i mouse_grid = WorldToTilePos(p->world_mouse);
    float dsq;
    v3fNearTransform(p->world_mouse, GAME_ITER(Unit, transform), &dsq);
    if (dsq < kDsqSelect) {
      // Highlight the unit that would be selected on mouse click
      rgg::RenderRectangle(TilePosToWorld(mouse_grid), kTileScale,
                           kDefaultRotation, v4f(1.0f, 1.0f, 1.0f, .45f));
    }

    if (!p->mod_placement) continue;
    v4f color;
    color = v4f(1.0f, 0.0f, 1.f, 1.0f);
    rgg::RenderRectangle(TilePosToWorld(mouse_grid), kTileScale,
                         kDefaultRotation, color);
  }

  RenderCrew();

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    rgg::RenderTag(kGfx.asteroid_tag, asteroid->transform.position,
                   asteroid->transform.scale, kDefaultRotation, kWhite);
  }

  for (int i = 0, j = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];
    rgg::RenderTag(kGfx.missile_tag, missile->transform.position,
                   missile->transform.scale, kDefaultRotation, kWhite);

    v2i tile = WorldToTilePos(missile->transform.position);
    if (!TileOk(tile)) continue;

    for (; j < kUsedModule; ++j) {
      Module* mod = &kModule[j];
      if (mod->mkind != kModTurret) continue;
      v2f pos = TilePosToWorld(v2i(mod->cx, mod->cy));
      rgg::RenderLine(missile->transform.position, pos,
                      v4f(1.0f, 0.0, 0.0, 1.f));
    }
  }

  for (int i = 0; i < kUsedProjectile; ++i) {
    Projectile* p = &kProjectile[i];
    float radius = 2.0f;
    rgg::RenderLine(p->start, p->end,
                    v4f(1.0f, 0.45f, 0.23f, p->duration / 100.0f));
  }

  for (int i = 0; i < kUsedNotify; ++i) {
    Notify* n = &kNotify[i];
    float radius = 50.f - (n->age * 1.f);
    rgg::RenderCircle(n->position, radius - 10.f, radius, kWhite);
  }

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];
    rgg::RenderTag(kGfx.pod_tag, pod->transform.position, pod->transform.scale,
                   kDefaultRotation, kWhite);

    if (pod->think_flags & FLAG(kPodAiUnmanned)) continue;

    rgg::RenderRectangle(pod->transform.position + v2f(15.f, 15.f),
                         v3f(0.25f, 0.25f, 0.f), kDefaultRotation,
                         v4f(0.99f, 0.33f, 0.33f, 1.f));
    rgg::RenderCircle(pod->transform.position + v2f(15.f, 15.f), 12.f, 14.f,
                      v4f(0.99f, 0.33f, 0.33f, 1.f));
  }

  // Ui
  imui::Render();
}

}  // namespace gfx
