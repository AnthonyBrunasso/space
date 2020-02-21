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
constexpr int MAX_SELECTED_TEXT = CREWA_MAX + 2;
static char selected_text[MAX_SELECTED_TEXT][64];

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
Render(const math::Rectf visible_world, v2f mouse, v2f screen)
{
  using namespace simulation;

  const v2f grid2(50.f, 50.f);
  math::Rectf world2 = visible_world;
  AlignToGrid(grid2, &world2);
  rgg::RenderGrid(grid2, world2, v4f(0.207f, 0.317f, 0.360f, 0.60f));

  const v2f grid1(25.f, 25.f);
  math::Rectf world1 = visible_world;
  AlignToGrid(grid1, &world1);
  rgg::RenderGrid(grid1, world1, v4f(0.050f, 0.215f, 0.050f, 0.45f));

  // selected Unit text
  bool bSelected = false;
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (unit->kind != Unit::kPlayerControlled) continue;
    bSelected = true;

    int t = 0;
    for (; t < CREWA_MAX; ++t) {
      snprintf(selected_text[t], 64, "[%u,%u] %s", unit->aknown_min[t],
               unit->aknown_max[t], crew_aname[t]);
    }
    snprintf(selected_text[t++], 64, "(%04.02f,%04.02f)",
             unit->transform.position.x, unit->transform.position.y);
    snprintf(selected_text[t++], 64, "uaction: %d", unit->uaction);

    break;
  }

  // Base ship
  float sys_power = .0f;
  float sys_engine = .0f;
  float sys_mine = .0f;
  float sys_turret = 0.f;
  constexpr float min_visibility = .2f;
  if (kUsedShip) {
    sys_power = kShip[0].sys_power;

    sys_power = CLAMPF(sys_power, min_visibility, 1.0f);
    sys_engine = CLAMPF(kShip[0].sys_engine, min_visibility, sys_power);
    sys_mine = CLAMPF(kShip[0].sys_mine, min_visibility, sys_power);
    sys_turret = CLAMPF(kShip[0].sys_turret, min_visibility, sys_power);
  }

  static float escale = .1f;
  if (escale > 2.f) {
    escale = .1f;
  }

  for (int i = 0; i < kUsedModule; ++i) {
    Module* module = &kModule[i];
    v4f color;
    if (module->mod_mine) {
      color = v4f(0.0, 0.75f * sys_mine, 0.0f, 1.0f);
    } else if (module->mod_power) {
      color = v4f(0.0f, 0.0f, 0.75f * sys_power, 1.0f);
    } else if (module->mod_engine) {
      color = v4f(1.0f * sys_engine, 0.0f, 1.f * sys_engine, 1.0f);
    } else if (module->mod_turret) {
      color = v4f(1.f * sys_turret, 0.f, 0.f, 1.f);
    } else {
      continue;
    }
    rgg::RenderRectangle(
        v3f(simulation::TilePosToWorld(v2i{(int)module->cx, (int)module->cy})),
        v3f(1.f / 2.f, 1.f / 2.f, 1.f), kDefaultRotation, color);
  }

  {
    v3f world;
    for (int i = 0; i < kUsedModule; ++i) {
      Module* mod = &kModule[i];
      if (!mod->mod_engine) continue;
      v2i hack = {-1, 2};
      world = TilePosToWorld(v2i(mod->cx, mod->cy) + hack);
      v4f engine_color = v4f(1.f * sys_engine, 0.f, 1.f * sys_engine, 1.f);
      rgg::RenderTag(kGfx.exhaust_tag, world, v3f(escale, 1.f, 1.f),
                     kDefaultRotation, engine_color);
      rgg::RenderTag(kGfx.exhaust_tag, world,
                     v3f(fmodf(escale + 1.f, 2.f), 1.f, 1.f), kDefaultRotation,
                     engine_color);
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

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];

    if (unit->inspace) {
      continue;
    }

    const v3f* p = &unit->transform.position;
    v2f grid = TilePosToWorld(WorldToTilePos(p->xy()));

    v4f color;
    switch (unit->kind) {
      case 0:
        color = v4f(0.26f, 0.33f, 0.68f, 1.f);
        break;
      case 1:
        color = v4f(0.50f, .33f, .33f, 1.f);
        break;
      case 2:
        color = v4f(0.66f, 0.33f, 0.33f, 1.f);
        break;
      case 3:
        color = v4f(0.74f, 0.33f, 0.33f, 1.f);
        break;
      case 4:
        color = v4f(0.86f, 0.33f, 0.33f, 1.f);
        break;
      case 5:
        color = v4f(0.99f, 0.33f, 0.33f, 1.f);
        break;
      default:
        continue;
    }
    // Draw the player.
    rgg::RenderRectangle(unit->transform.position, unit->transform.scale,
                         kDefaultRotation, color);

    // Space suit
    if (unit->spacesuit) {
      rgg::RenderCircle(unit->transform.position, 12.f, 14.f,
                        v4f(0.99f, 0.33f, 0.33f, 1.f));
    }

    v4f hilite;
    switch (unit->kind) {
      case 0:
        hilite = v4f(1.f, 0.f, 0.f, .45f);
        break;
      default:
        hilite = v4f(5.f, 5.f, 5.f, .45f);
        break;
    };

    if (v3fDsq(unit->transform.position, mouse) < kDsqSelect) {
      // Highlight the unit that would be selected on mouse click
      rgg::RenderRectangle(v3f(grid), v3f(1.f / 2.f, 1.f / 2.f, 1.f),
                           kDefaultRotation, hilite);
    }

    if (unit->uaction != kUaMove) continue;

    // Show the path they are on if they have one.
    v2i start = WorldToTilePos(p->xy());
    v2i end = WorldToTilePos(unit->data.destination);

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

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    rgg::RenderTag(kGfx.asteroid_tag, asteroid->transform.position,
                   asteroid->transform.scale, kDefaultRotation, kWhite);
  }

  for (int i = 0, j = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];
    rgg::RenderTag(kGfx.missile_tag, missile->transform.position,
                   missile->transform.scale, kDefaultRotation, kWhite);

    v2i tile = WorldToTilePos(missile->transform.position.xy());
    if (!TileOk(tile)) continue;

    for (; j < kUsedModule; ++j) {
      Module* mod = &kModule[j];
      if (!mod->mod_turret) continue;
      v2f pos = TilePosToWorld(v2i(mod->cx, mod->cy));
      rgg::RenderLine(missile->transform.position, pos,
                      v4f(1.0f, 0.0, 0.0, 1.f));
    }
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

  if (bSelected) {
    imui::Begin(v2f(screen.x - 225.f, screen.y - 30.0f));
    for (int i = 0; i < MAX_SELECTED_TEXT; ++i) {
      imui::Text(selected_text[i]);
    }
    imui::End();
  }

  // Ui
  imui::Render();
}

}  // namespace gfx
