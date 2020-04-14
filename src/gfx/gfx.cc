#pragma once

#include "imui.cc"
#include "math/vec.h"
#include "platform/platform.cc"
#include "renderer/renderer.cc"

#include "../simulation/simulation.cc"

namespace gfx
{
struct Gfx {
  RenderTag cryo_tag;
  RenderTag exhaust_tag;
  RenderTag plus_tag;
};

static Gfx kGfx;
static v4f kSelectionColor = v4f(0.19f, 0.803f, 0.19f, 0.40f);
static v4f kSelectionOutlineColor = v4f(0.19f, 0.803f, 0.19f, 1.f);
static v4f kRed = v4f(1.f, 0.f, 0.f, 1.f);
static v4f kGreen = v4f(0.f, 1.f, 0.f, 1.f);
static v4f kBlue = v4f(0.f, 0.f, 1.f, 1.f);
static v4f kWhite = v4f(1.f, 1.f, 1.f, 1.f);
static v4f kGray = v4f(.6f, .6f, .6f, 1.f);
static const math::Quatf kDefaultRotation = math::Quatf(0.f, 0.f, 0.f, 1.f);
static v3f kDefaultScale = v3f(1.f, 1.f, 1.f);
static v3f kTileScale = v3f(0.5f, 0.5f, 1.f);
static bool kRenderGrid = false;
static bool kRenderPath = false;

struct DebugCube {
  math::Cubef cube;
  v4f color;
};

DECLARE_ARRAY(DebugCube, 64);

bool
Initialize(const window::CreateInfo& window_create_info)
{
  int window_result = window::Create("Space", window_create_info);
  printf("Window create result: %i\n", window_result);
  auto status = rgg::Initialize();
  constexpr int kVertCount = 29;
  constexpr int kFloatCount = kVertCount * 3;
  // clang-format off
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
  for (int i = 0; i < kCryoVert * 3; ++i) cryo[i] *= 12.f;        // HA
  for (int i = 0; i < kExhaustVert * 3; ++i) exhaust[i] *= 15.f;  // HA
  for (int i = 0; i < kPlusVert * 3; ++i) plus[i] *= 15.f;        // HA
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
  FOR_EACH_ENTITY(Unit, unit, {
    if (unit->ship_index != ship_index) continue;
    if (unit->inspace) {
      rgg::RenderPod(unit->position, v3f(20.f, 20.f, 20.f),
                     math::Quatf(-90.f, v3f(1.f, 0.f, 0.f)),
                     v4f(1.f, 1.f, 1.f, 1.f));

      continue;
    }
    if (unit->tile.shroud && !unit->tile.visible) continue;
    if (unit->notify) {
      const float radius = 50.f - (unit->notify * 1.f);
      rgg::RenderCircle(unit->position, radius - 10.f, radius, kWhite);
    }

    v4f color;
    switch (unit->kind) {
      case kOperator: {
        switch (unit->player_index) {
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
      rgg::RenderCircle(unit->position + v3f(0.f, 0.f, 0.08f), 12.f, 14.f,
                        v4f(0.33f, 0.80f, 0.33f, 1.f));
    }

    FOR_EACH_ENTITY(Module, mod, {
      if (v3fDsq(unit->position, mod->position) < kDsqOperate) {
        // TODO(abrunasso): This should be the graphic when working on
        // something.
        static float r = 0.f;
        rgg::RenderGear(
            unit->position + v3f(-unit->bounds.x - 6.f, unit->bounds.y + 2.f,
                                 unit->bounds.z + .1f),
            v3f(8.f, 8.f, 8.f), math::Quatf(r, v3f(0.f, 0.f, 1.f)),
            v4f(0.7f, 0.7f, 0.7f, 1.f));
        r += 0.3f;
      }
    });

    v3f crew_bounds = v3f(10.f, 10.f, 10.f);
    const int* behavior;
    if (BB_GET(unit->bb, kUnitBehavior, behavior) &&
        *behavior == kUnitBehaviorCrewMember) {
      crew_bounds = v3f(8.f, 8.f, 8.f);
      color = v4f(color.x * .8f, color.y * .8f, color.z * .8f, color.w);
    }
    rgg::RenderCrew(unit->position + v3f(0.f, 0.f, 20.f), crew_bounds,
                    math::Quatf(-90, v3f(0.f, 0.f, 1.f)), color);

    if (unit->spacesuit) {
      // rgg::RenderSphere(unit->position + v3f(0.f, 0.f, 7.5f),
      //                  v3f(15.f, 15.f, 15.f), v4f(1.f, 1.f, 1.f, 0.3f));
    }

    // Render unit health bars.
    static const float kHealthSz = 5.f;
    v3f hstart = unit->position + v3f(-13.0f, 15.5f, 0.0f);
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
          Rectf(hstart.x, hstart.y, kHealthSz, kHealthSz), unit->bounds.z,
          hcolor);
      rgg::RenderLineRectangle(
          Rectf(hstart.x, hstart.y, kHealthSz, kHealthSz), unit->bounds.z,
          v4f(0.3f, 0.3f, 0.3f, 1.0f));
      hstart.x += kHealthSz;
    }
  });

  if (kRenderPath) {
    FOR_EACH_ENTITY(Unit, unit, {
      if (unit->uaction != kUaMove) continue;
      if (unit->inspace) continue;

      // Show the path they are on if they have one.
      const Tile* dest = nullptr;
      if (!BB_GET(unit->bb, kUnitDestination, dest)) continue;

      auto* path = PathTo(unit->tile, *dest);
      if (!path || path->size <= 1) {
        continue;
      }

      for (int i = 0; i < path->size; ++i) {
        v2f world_pos = FromShip(path->tile[i]).Center();
        rgg::RenderRectangle(world_pos, v3f(1.f / 3.f, 1.f / 3.f, 1.f),
                             kDefaultRotation, v4f(0.33f, 0.33f, 0.33f, 0.40f));
      }
    });
  }
}

void
RenderShip(uint64_t ship_index)
{
  using namespace simulation;

  // Modules are always visible
  FOR_EACH_ENTITY(Module, mod, {
    v3f mcolor = ModuleColor(mod->mkind);
    v4f color(mcolor.x, mcolor.y, mcolor.z, 1.f);
    if (ModuleBuilt(mod)) {
      rgg::RenderCube(
          math::Cubef(mod->position + v3f(0.f, 0.f, 15.f / 2.f), mod->bounds),
          color);
    } else {
      rgg::RenderLineCube(
          math::Cubef(mod->position + v3f(0.f, 0.f, 15.f / 2.f), mod->bounds),
          v4f(color.x, color.y, color.z, 1.f));
      glDisable(GL_DEPTH_TEST);
      rgg::RenderProgressBar(
          Rectf(mod->position.x - mod->bounds.x / 2.f,
                      mod->position.y - mod->bounds.y, mod->bounds.y, 5.f),
          2.f, mod->frames_building, mod->frames_to_build, kGreen, kWhite);
      glEnable(GL_DEPTH_TEST);
    }

    if (mod->frames_training == kTrainIdle) continue;

    glDisable(GL_DEPTH_TEST);
    rgg::RenderProgressBar(
        Rectf(mod->position.x - mod->bounds.x / 2.f,
                    mod->position.y - mod->bounds.y, mod->bounds.y, 5.f),
        2.f, mod->frames_training, mod->frames_to_train, kRed, kWhite);
    glEnable(GL_DEPTH_TEST);
  });

  {
    v3f world;
    for (int i = 0; i < kUsedEntity; ++i) {
      Module* mod = i2Module(i);
      if (!mod) continue;
      if (mod->ship_index != ship_index) continue;
      if (mod->mkind != kModEngine) continue;
      if (!ModuleBuilt(mod)) continue;
      v3f mcolor = ModuleColor(mod->mkind);
      v2f hack = {-kTileWidth, kTileHeight};
      world = mod->position + hack;
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

  float fft = kShip[ship_index].ftl_frame * (1.f / kFtlFrameTime);
  float ship_alpha = 1.f - fft;
  uint16_t ship_width = kShip[ship_index].map_width;
  uint16_t ship_height = kShip[ship_index].map_height;
  for (int i = 0; i < ship_height; ++i) {
    for (int j = 0; j < ship_width; ++j) {
      const Tile* tile = ShipTile(ship_index, j, i);
      v2f world_pos = FromShip(*tile).Center();

      v4f color;
      if (tile->shroud && !tile->explored) {
        color = v4f(0.3f, 0.3f, 0.3f, .7);
        rgg::RenderRectangle(world_pos, kTileScale, kDefaultRotation, color);
      } else if (tile->blocked) {
        color = v4f(.15f, .15f, .15f, 1.f);
        rgg::RenderCube(math::Cubef(v3f(0.f, 0.f, 50.f) + world_pos, kTileWidth,
                                    kTileHeight, 100.f),
                        color);
      } else if (tile->nooxygen) {
        color = v4f(1.f, 0.f, .2f, .4);
        rgg::RenderRectangle(world_pos, kTileScale, kDefaultRotation, color);
      } else if (tile->shroud && !tile->visible) {
        color = v4f(0.3f, 0.3f, 0.3f, .4);
        rgg::RenderRectangle(world_pos, kTileScale, kDefaultRotation, color);
      }
    }
  }

  for (int i = 0; i < kUsedPlayer; ++i) {
    Player* p = &kPlayer[i];
    if (!TileValid(p->mouse_tile)) continue;

    switch (p->hud_mode) {
      case kHudSelection: {
        if (p->selection_start.x != 0.f || p->selection_start.y != 0.f ||
            p->selection_start.z != 0.f) {
          glDisable(GL_DEPTH_TEST);
          v3f diff = p->mouse_world - p->selection_start;
          Rectf sbox(p->selection_start.x, p->selection_start.y, diff.x,
                           diff.y);
          sbox = math::OrientToAabb(sbox);
          rgg::RenderRectangle(sbox, p->mouse_world.z, kSelectionColor);
          rgg::RenderLineRectangle(sbox, p->mouse_world.z,
                                   kSelectionOutlineColor);
          glEnable(GL_DEPTH_TEST);
        }
      } break;
      case kHudModule: {
        v3f mcolor = ModuleColor(p->mod_placement);
        v4f color(mcolor.x, mcolor.y, mcolor.z, 1.f);
        v3f bounds = ModuleBounds(p->mod_placement);
        v3f mgw = FromShip(p->mouse_tile).Center();
        rgg::RenderLineCube(math::Cubef(mgw + v3f(0.f, 0.f, bounds.z / 2.f),
                                        bounds.x, bounds.y, bounds.z),
                            color);
      } break;
      case kHudAttackMove: {
        glDisable(GL_DEPTH_TEST);
        rgg::RenderTag(kGfx.plus_tag, p->mouse_world, kDefaultScale,
                       kDefaultRotation, v4f(1.f, 0.f, 0.f, 1.f));
        glEnable(GL_DEPTH_TEST);
      } break;
    };
  }

  if (kRenderGrid) {
    const v2f grid_dims(kTileWidth, kTileHeight);
    v4f colors[] = {
        v4f(0.207f, 0.317f, 0.360f, 0.60f),
        v4f(0.050f, 0.215f, 0.050f, 0.45f),
    };
    rgg::RenderGrid(grid_dims, ShipBounds(ship_index), ARRAY_LENGTH(colors),
                    colors);
  }
}

void
RenderSpaceObjects()
{
  using namespace simulation;

  // Stuff
  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    static float r = 0.1f;
    static float nr = .1f;
    rgg::RenderAsteroid(asteroid->transform.position, v3f(20.f, 20.f, 20.f),
                        math::Quatf(90.f + r, v3f(nr, 0.f, 1.f - nr)),
                        v4f(.4f, .4f, .4f, 1.f));
    r += .3f;
    nr += .05f;
  }

  for (int i = 0; i < kUsedProjectile; ++i) {
    Projectile* p = &kProjectile[i];
    float radius = 2.0f;
    float progress = (float)p->frame / p->duration;
    float inverse_progress = 1.0 - progress;

    switch (p->wkind) {
      case kWeaponMiningLaser: {
        rgg::RenderLine(p->start, p->end, v4f(0.40f, 1.0f, 0.23f, 1.0));
      } break;
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

  for (int i = 0; i < kUsedInvasion; ++i) {
    Invasion* v = &kInvasion[i];
    rgg::RenderCube(math::Cubef(v->transform.position + v3f(0.f, 0.f, 5.f),
                                20.f, 20.f, 20.f),
                    v4f(.7f, .1f, .1f, 1.f));
  }
}

void
Render(uint64_t player_index)
{
  rgg::kObserver.position = kPlayer[player_index].camera.position;

  for (int i = 0; i < kUsedShip; ++i) {
    if (kShip[i].level != kPlayer[player_index].level) continue;

    RenderShip(i);
    RenderCrew(i);
  }

  RenderSpaceObjects();

  // Render debug graphics.
  for (int i = 0; i < kUsedDebugCube; ++i) {
    rgg::RenderLineCube(kDebugCube[i].cube, kDebugCube[i].color);
  }

  // Ui
  imui::Render(player_index);
  imui::Render(imui::kEveryoneTag);
}

void
Reset()
{
  kUsedDebugCube = 0;
}

void
PushDebugCube(const math::Cubef& cube, const v4f& color)
{
  DebugCube* dcube = UseDebugCube();
  dcube->cube = cube;
  dcube->color = color;
}

}  // namespace gfx
