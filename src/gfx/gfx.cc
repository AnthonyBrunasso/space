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
  RenderTag missile_tag;
  RenderTag cryo_tag;
  RenderTag exhaust_tag;
  RenderTag plus_tag;
};

static Gfx kGfx;
static v4f kWhite = v4f(1.f, 1.f, 1.f, 1.f);
static v4f kSelectionColor = v4f(0.19f, 0.803f, 0.19f, 0.40f);
static v4f kSelectionOutlineColor = v4f(0.19f, 0.803f, 0.19f, 1.f);
static v4f kRed = v4f(1.f, 0.f, 0.f, 1.f);
static const math::Quatf kDefaultRotation = math::Quatf(0.f, 0.f, 0.f, 1.f);
static v3f kDefaultScale = v3f(1.f, 1.f, 1.f);
static v3f kTileScale = v3f(0.5f, 0.5f, 1.f);

bool
Initialize()
{
  int window_result = window::Create("Space", 1920, 1080, false);
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
  constexpr int kMissileVert = 4;
  GLfloat missile[kMissileVert*3] = 
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
  constexpr int kPlusVert = 8;
#define ORIGIN 0.0f, 0.0f, 0.0f
  GLfloat plus[kPlusVert*3] = 
  {-1.0f, 0.f, 0.f, ORIGIN,
    0.0f, -1.0f, 0.0f, ORIGIN,
    1.0f, 0.0f, 0.0f, ORIGIN, 
    0.0f, 1.0f, 0.0f, ORIGIN};
  // clang-format on
  for (int i = 0; i < kFloatCount; ++i) asteroid[i] *= 15.f;      // HA
  for (int i = 0; i < kMissileVert * 3; ++i) missile[i] *= 15.f;  // HA
  for (int i = 0; i < kCryoVert * 3; ++i) cryo[i] *= 12.f;        // HA
  for (int i = 0; i < kExhaustVert * 3; ++i) exhaust[i] *= 15.f;  // HA
  for (int i = 0; i < kPlusVert * 3; ++i) plus[i] *= 15.f;        // HA
  kGfx.asteroid_tag = rgg::CreateRenderable(kVertCount, asteroid, GL_LINE_LOOP);
  kGfx.missile_tag = rgg::CreateRenderable(kMissileVert, missile, GL_LINE_LOOP);
  kGfx.cryo_tag = rgg::CreateRenderable(kCryoVert, cryo, GL_LINE_LOOP);
  kGfx.exhaust_tag = rgg::CreateRenderable(kExhaustVert, exhaust, GL_LINE_LOOP);
  kGfx.plus_tag = rgg::CreateRenderable(kPlusVert, plus, GL_LINE_LOOP);
  return status;
}

void
RenderCrew(uint64_t ship_index)
{
  using namespace simulation;

  // Crew rendering
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (unit->ship_index != ship_index) continue;
    if (unit->inspace) {
      rgg::RenderPod(unit->transform.position, v3f(20.f, 20.f, 20.f),
          math::Quatf(-90.f, v3f(1.f, 0.f, 0.f)), v4f(1.f, 1.f, 1.f, 1.f));

      continue;
    }
    Tile* tile = TilePtr(WorldToTilePos(unit->transform.position));
    if (tile && tile->shroud) continue;
    if (unit->notify) {
      const float radius = 50.f - (unit->notify * 1.f);
      rgg::RenderCircle(unit->transform.position, radius - 10.f, radius, kWhite);
    }

    v4f color;
    switch (unit->kind) {
      case kOperator: {
        switch (unit->player_id) {
          case 0:
            color = v4f(0.70f, .33f, .33f, 1.f);
            break;
          case 1:
            color = v4f(0.33f, .33f, .70f, 1.f);
            break;
          case 2:
            color = v4f(0.33f, .33f, .33f, 1.f);
            break;
          default:
            color = v4f(1.f, 1.f, 1.f, 1.f);
            break;
        }
      } break;
      case kAlien:
        color = v4f(0.30, 0.70f, 0.10f, 1.f);
        break;
      default:
        continue;
    }

    if (unit->control & (1 << kPlayerIndex)) {
        rgg::RenderCircle(unit->transform.position + v3f(0.f, 0.f, 0.08f),
                          12.f, 14.f, v4f(0.33f, 0.80f, 0.33f, 1.f));
    }

    for (int k = 0; k < kUsedModule; ++k) {
      Module* mod = &kModule[k];
      if (v3fDsq(unit->transform.position, v3fModule(mod)) < kDsqOperate) {
        // TODO(abrunasso): This should be the graphic when working on something.
        static float r = 0.f;
        rgg::RenderGear(
            unit->transform.position +
                v3f(-unit->bounds.x - 6.f, unit->bounds.y + 2.f, unit->bounds.z + .1f),
            v3f(8.f, 8.f, 8.f),
            math::Quatf(r, v3f(0.f, 0.f, 1.f)), v4f(0.7f, 0.7f, 0.7f, 1.f));
        r += 0.3f;
      }
    }

    rgg::RenderCube(math::Cubef(unit->transform.position +
                                    v3f(0.f, 0.f, unit->bounds.z / 2.f),
                                unit->bounds),
                    color);

    if (unit->spacesuit) {
      //rgg::RenderSphere(unit->transform.position + v3f(0.f, 0.f, 7.5f),
      //                  v3f(15.f, 15.f, 15.f), v4f(1.f, 1.f, 1.f, 0.3f));
    }


    // Render unit health bars.
    static const float kHealthSz = 5.f;
    v3f hstart = unit->transform.position + v3f(-13.0f, 15.5f, 0.0f);
    float hratio = unit->health / unit->max_health;
    v4f hcolor = v4f(0.0f, 1.0f, 0.0f, 1.0f);
    if (hratio > 0.35f && hratio < 0.75f) {
      hcolor = v4f(0.898f, 0.425f, 0.0f, 1.0f);
    } else if (hratio <= 0.35f) {
      hcolor = v4f(1.0f, 0.1f, 0.0f, 1.0f);
    }

    for (int i = 1; i < 6; ++i) {
      float bar = (float)i;
      float scaled_max =
          math::ScaleRange(unit->max_health, 5.f, unit->max_health);
      float scaled_health =
          math::ScaleRange(unit->health, 5.f, unit->max_health);
      if (bar > scaled_health) {
        float bar_range = unit->max_health / 5.f;
        float hdiff = bar_range * bar - unit->health;
        hcolor.w = math::ScaleRange(hdiff, 0.f, bar_range, 1.f, 0.f);
      }
      rgg::RenderRectangle(
          math::Rectf(hstart.x, hstart.y, kHealthSz, kHealthSz), unit->bounds.z,
          hcolor);
      rgg::RenderLineRectangle(
          math::Rectf(hstart.x, hstart.y, kHealthSz, kHealthSz), unit->bounds.z,
          v4f(0.3f, 0.3f, 0.3f, 1.0f));
      hstart.x += kHealthSz;
    }
  }

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (unit->uaction != kUaMove) continue;
    if (unit->inspace) continue;

    // Show the path they are on if they have one.
    v2i start = WorldToTilePos(unit->transform.position);
    const v3f* dest = nullptr;
    if (!BB_GET(unit->bb, kUnitDestination, dest)) continue;
    v2i end = WorldToTilePos(*dest);

    auto* path = PathTo(start, end);
    if (!path || path->size <= 1) {
      continue;
    }

    for (int i = 0; i < path->size; ++i) {
      v2i pos = path->tile[i];
      v2f world_pos = TilePosToWorld(pos);
      rgg::RenderRectangle(world_pos, v3f(1.f / 3.f, 1.f / 3.f, 1.f),
                           kDefaultRotation, v4f(0.33f, 0.33f, 0.33f, 0.40f));
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
    case kModBarrack:
      return v3f(1.0, 1.0, 1.0);
  }

  return v3f();
}

void
RenderShip(uint64_t ship_index)
{
  using namespace simulation;

  // Modules are always visible
  constexpr float min_visibility = .4f;

  for (int i = 0; i < kUsedModule; ++i) {
    Module* module = &kModule[i];
    if (module->ship_index != ship_index) continue;
    if (!module->built) continue;
    v3f mcolor = ModuleColor(module->mkind);
    mcolor *=
        CLAMPF(min_visibility, kShip[ship_index].sys[module->mkind], 1.0f);
    v4f color(mcolor.x, mcolor.y, mcolor.z, 1.f);
    v2f t = simulation::TilePosToWorld(v2i{(int)module->cx, (int)module->cy});
    rgg::RenderCube(math::Cubef(v3f(t.x, t.y, 0.f) + v3f(0.f, 0.f, 15.f / 2.f),
                                15.f, 15.f, 15.f),
                    color);
  }

  {
    v3f world;
    for (int i = 0; i < kUsedModule; ++i) {
      Module* mod = &kModule[i];
      if (mod->ship_index != ship_index) continue;
      if (mod->mkind != kModEngine) continue;
      if (!mod->built) continue;
      float engine_power = fminf(kShip[ship_index].sys[kModEngine],
                                 kShip[ship_index].sys[kModPower]);
      float visibility = fmaxf(min_visibility, engine_power);
      v3f mcolor = ModuleColor(mod->mkind);
      mcolor *= visibility;
      v2i hack = {-1, 1};
      world = TilePosToWorld(v2i(mod->cx, mod->cy) + hack);
      v4f color(mcolor.x, mcolor.y, mcolor.z, 1.f);
      float engine_scale = kShip[ship_index].engine_animation * .1f;
      rgg::RenderExhaust(world + v3f(-40.f, -20.f, 0.f), v3f(22.f, 22.f, 22.f),
                        math::Quatf(180, v3f(0.f, 0.f, 1.f)),
                        v4f(0.4f, 0.4f, 0.4f, 1.f));
      rgg::RenderTag(kGfx.exhaust_tag, world + v3f(-50.f, 3.f, 0.f),
                     v3f(engine_scale, 1.f, 1.f), kDefaultRotation, color);
      rgg::RenderTag(kGfx.exhaust_tag, world + v3f(-50.f, 3.f, 0.f),
                     v3f(fmodf(engine_scale + 0.8f, 1.6f), 1.f, 1.f),
                     kDefaultRotation, color);
    }
  }

  for (int i = 0; i < kUsedConsumable; ++i) {
    Consumable* c = &kConsumable[i];
    if (c->ship_index != ship_index) continue;
    const Tile* tile = TilePtr(v2i(c->cx, c->cy));
    if (!tile || !tile->explored) continue;

    v4f color = v4f(.3f, .3f, .7f, 1.f);
    v3f world = TileToWorld(*tile);
    if (c->cryo_chamber) {
      rgg::RenderTag(kGfx.cryo_tag, world, kTileScale, kDefaultRotation, color);
    } else {
      rgg::RenderSphere(world + v3f(0.f, 0.f, 5.f), v3f(7.f, 7.f, 7.f), color);
    }
  }

  float fft = kShip[ship_index].ftl_frame * (1.f / kFtlFrameTime);
  float ship_alpha = 1.f - fft;
  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      const Tile* tile = TilePtr(v2i(j, i));
      v3f world_pos = TileToWorld(*tile);
      // printf("%.3f\n", world_pos.z);

      v4f color;
      if (!tile->explored) {
        color = v4f(0.3f, 0.3f, 0.3f, .7);
        rgg::RenderRectangle(world_pos, kTileScale, kDefaultRotation, color);
      } else if (tile->blocked) {
        color = v4f(.15f, .15f, .15f, 1.f);
        rgg::RenderCube(math::Cubef(world_pos + v3f(0.f, 0.f, 50.f), kTileWidth,
                                    kTileHeight, 100.f),
                        color);
      } else if (tile->nooxygen) {
        color = v4f(1.f, 0.f, .2f, .4);
        rgg::RenderRectangle(world_pos, kTileScale, kDefaultRotation, color);
      } else if (tile->shroud) {
        color = v4f(0.3f, 0.3f, 0.3f, .4);
        rgg::RenderRectangle(world_pos, kTileScale, kDefaultRotation, color);
      }
    }
  }

  for (int i = 0; i < kUsedPlayer; ++i) {
    Player* p = &kPlayer[i];
    v2i mouse_grid = WorldToTilePos(p->world_mouse);

    switch (p->hud_mode) {
      case kHudSelection: {
        if (p->selection_start.x != 0.f || p->selection_start.y != 0.f ||
            p->selection_start.z != 0.f) {
          glDisable(GL_DEPTH_TEST);
          v3f diff = p->world_mouse - p->selection_start;
          math::Rectf sbox(p->selection_start.x, p->selection_start.y, diff.x,
                           diff.y);
          sbox = math::OrientToAabb(sbox);
          rgg::RenderRectangle(sbox, p->world_mouse.z, kSelectionColor);
          rgg::RenderLineRectangle(sbox, p->world_mouse.z,
                                   kSelectionOutlineColor);
          glEnable(GL_DEPTH_TEST);
        }

        if (!TileOk(mouse_grid)) continue;

        float dsq;
        v3fNearTransform(p->world_mouse, GAME_ITER(Unit, transform), &dsq);
        if (dsq < kDsqSelect) {
          // Highlight the unit that would be selected on mouse click
          rgg::RenderRectangle(TilePosToWorld(mouse_grid), kTileScale,
                               kDefaultRotation, v4f(1.0f, 1.0f, 1.0f, .45f));
        }
      } break;
      case kHudModule: {
        for (int j = 0; j < kUsedModule; ++j) {
          Module *module = &kModule[j];
          if (module->mkind != p->mod_placement) continue;
          if (module->built) continue;
          v2i tile(module->cx, module->cy);

          // Cursor hover matches a module location
          v3f mcolor = ModuleColor(module->mkind);
          if (tile == mouse_grid)
            mcolor *= 1.f;
          else
            mcolor *= .3f;

          v4f color(mcolor.x, mcolor.y, mcolor.z, 1.f);
          v2f t = simulation::TilePosToWorld(tile);
          rgg::RenderCube(
              math::Cubef(v3f(t.x, t.y, 0.f) + v3f(0.f, 0.f, 15.f / 2.f), 15.f,
                          15.f, 15.f),
              color);
        }
      } break;
      case kHudAttackMove: {
        glDisable(GL_DEPTH_TEST);
        rgg::RenderTag(kGfx.plus_tag, p->world_mouse, kDefaultScale,
                       kDefaultRotation, v4f(1.f, 0.f, 0.f, 1.f));
        glEnable(GL_DEPTH_TEST);
      } break;
    };
  }

  const v2f grid_dims(kTileWidth, kTileHeight);
  v4f colors[] = {
      v4f(0.207f, 0.317f, 0.360f, 0.60f),
      v4f(0.050f, 0.215f, 0.050f, 0.45f),
  };
  rgg::RenderGrid(grid_dims, TilemapWorldBounds(), ARRAY_LENGTH(colors),
                  colors);
}

void
RenderSpaceObjects()
{
  using namespace simulation;

  // Stuff
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
    float progress = (float)p->frame / p->duration;
    float inverse_progress = 1.0 - progress;

    switch (p->wkind) {
      case kWeaponLaser: {
        rgg::RenderLine(p->start, p->end,
                        v4f(1.0f, 0.45f, 0.23f, inverse_progress));
      } break;
      case kWeaponBullet: {
        v3f dir = Normalize(p->end - p->start);
        v3f location = p->start + (dir * p->frame);
        rgg::RenderCircle(location, 5.0f, kWhite);
      } break;
      case kWeaponCount:
        break;
    }
  }
}

void
Render(uint64_t player_index)
{
  rgg::kObserver.position = kPlayer[player_index].camera.position;

  for (int i = 0; i < kUsedShip; ++i) {
    if (kShip[i].level != kPlayer[player_index].level) continue;

    simulation::TilemapSet(kShip[i].grid_index);
    RenderShip(i);
    RenderCrew(i);
  }

  RenderSpaceObjects();
  
  // Ui
  imui::Render(player_index);
}

}  // namespace gfx
