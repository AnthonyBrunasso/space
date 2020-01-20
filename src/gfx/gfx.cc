#include "gfx.h"

#include "renderer/renderer.cc"

#include "../simulation/simulation.cc"

namespace gfx
{
constexpr int kMaxTextSize = 128;
constexpr int kMaxTextCount = 32;

struct Text {
  char msg[kMaxTextSize];
  float screen_x;
  float screen_y;
};

struct Gfx {
  // Allow 32 on screen text messages.
  Text text[kMaxTextCount];
  int text_count;
  rgg::Tag asteroid_tag;
  rgg::Tag pod_tag;
  rgg::Tag missile_tag;
  math::AxisAlignedRect asteroid_aabb;
};

static Gfx kGfx;
static math::Vec4f kWhite = math::Vec4f(1.f, 1.f, 1.f, 1.f);

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
  // clang-format on
  for (int i = 0; i < kFloatCount; ++i) asteroid[i] *= 15.f;      // HA
  for (int i = 0; i < kPodVert * 3; ++i) pod[i] *= 15.f;          // HA
  for (int i = 0; i < kMissileVert * 3; ++i) missile[i] *= 15.f;  // HA
  kGfx.asteroid_aabb.min = math::Vec3f(10000.0f, 10000.0f, 10000.0f);
  kGfx.asteroid_aabb.max = math::Vec3f(-10000.0f, -10000.0f, -10000.0f);
  for (int i = 0; i < kFloatCount; i += 3) {
    auto& aabb = kGfx.asteroid_aabb;
    float x = asteroid[i];
    float y = asteroid[i];
    float z = asteroid[i];
    if (x < aabb.min.x) aabb.min.x = x;
    if (x > aabb.max.x) aabb.max.x = x;
    if (y < aabb.min.y) aabb.min.y = y;
    if (y > aabb.max.y) aabb.max.y = y;
    if (z < aabb.min.z) aabb.min.z = z;
    if (z > aabb.max.z) aabb.max.z = z;
  }
  kGfx.asteroid_tag = rgg::CreateRenderable(kVertCount, asteroid, GL_LINE_LOOP);
  kGfx.pod_tag = rgg::CreateRenderable(kPodVert, pod, GL_LINE_LOOP);
  kGfx.missile_tag = rgg::CreateRenderable(kMissileVert, missile, GL_LINE_LOOP);
  return status;
}

void
AlignToGrid(math::Vec2f grid, math::Rectf* world)
{
  float x_align = fmodf(world->min.x, grid.x);
  float y_align = fmodf(world->min.y, grid.y);

  world->min.x -= x_align + grid.x;
  world->min.y -= y_align + grid.y;
}

void
Reset()
{
  kGfx.text_count = 0;
}

void
Render(const math::Rectf visible_world, math::Vec2f mouse, math::Vec2f screen)
{
  rgg::RenderButton("test", math::Rect(10, 10, 50, 50));

  // Unit hover-over text
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];
    if (dsq(unit->transform.position, mouse) >= simulation::kDsqSelect)
      continue;

    char buffer[64];
    for (int j = 0; j < CREWA_MAX; ++j) {
      sprintf(buffer, "%u < %s < %u", unit->aknown_min[j], crew_aname[j],
              unit->aknown_max[j]);
      rgg::RenderText(buffer, screen.x - 225.f, screen.y - j * 25.f,
                      math::Vec4f());
    }
    sprintf(buffer, "%04.02fX %04.02fY", unit->transform.position.x,
            unit->transform.position.y);
    rgg::RenderText(buffer, screen.x - 225.f, screen.y - CREWA_MAX * 25.f,
                    math::Vec4f());
    break;
  }

  // Draw all text.
  for (int i = 0; i < kGfx.text_count; ++i) {
    Text& text = kGfx.text[i];
    rgg::RenderText(text.msg, text.screen_x, text.screen_y, math::Vec4f());
  }

  using namespace tilemap;

  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];

    const math::Vec3f* p = &unit->transform.position;
    math::Vec2f grid = TilePosToWorld(WorldToTilePos(p->xy()));

    math::Vec4f color;
    switch (unit->kind) {
      case 0:
        color = math::Vec4f(0.26f, 0.33f, 0.68f, 1.f);
        break;
      case 1:
        color = math::Vec4f(0.66f, 0.33f, 0.33f, 1.f);
        break;
      case 2:
        color = math::Vec4f(0.86f, 0.33f, 0.33f, 1.f);
        break;
      case 3:
        color = math::Vec4f(0.99f, 0.33f, 0.33f, 1.f);
        break;
      default:
        break;
    }
    // Draw the player.
    rgg::RenderRectangle(unit->transform.position, unit->transform.scale,
                         unit->transform.orientation, color);

    math::Vec4f hilite;
    switch (unit->kind) {
      case 0:
        hilite = math::Vec4f(1.f, 0.f, 0.f, .45f);
        break;
      default:
        hilite = math::Vec4f(5.f, 5.f, 5.f, .45f);
        break;
    };

    if (dsq(unit->transform.position, mouse) < simulation::kDsqSelect) {
      // Highlight the unit that would be selected on mouse click
      rgg::RenderRectangle(math::Vec3f(grid),
                           math::Vec3f(1.f / 2.f, 1.f / 2.f, 1.f),
                           math::Quatf(0.f, 0.f, 0.f, 1.f), hilite);
    }

    if (unit->command.type != Command::kMove) continue;

    // Show the path they are on if they have one.
    math::Vec2i start = WorldToTilePos(p->xy());
    math::Vec2i end = WorldToTilePos(unit->command.destination);

    auto* path = search::PathTo(start, end);
    if (!path || path->size <= 1) {
      continue;
    }

    for (int i = 0; i < path->size; ++i) {
      auto* t = &path->tile[i];
      rgg::RenderRectangle(math::Vec3f(TilePosToWorld(*t)),
                           math::Vec3f(1.f / 3.f, 1.f / 3.f, 1.f),
                           math::Quatf(0.f, 0.f, 0.f, 1.f),
                           math::Vec4f(0.33f, 0.33f, 0.33f, 0.40f));
    }
  }

  for (int i = 0; i < kUsedPod; ++i) {
    Pod* pod = &kPod[i];
    rgg::RenderTag(kGfx.pod_tag, pod->transform.position, pod->transform.scale,
                   pod->transform.orientation, kWhite);
  }

  for (int i = 0; i < kUsedAsteroid; ++i) {
    Asteroid* asteroid = &kAsteroid[i];
    rgg::RenderTag(kGfx.asteroid_tag, asteroid->transform.position,
                   asteroid->transform.scale, asteroid->transform.orientation,
                   kWhite);
  }

  for (int i = 0; i < kUsedMissile; ++i) {
    Missile* missile = &kMissile[i];
    rgg::RenderTag(kGfx.missile_tag, missile->transform.position,
                   missile->transform.scale, missile->transform.orientation,
                   kWhite);
  }

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

  for (int i = 0; i < kMapHeight; ++i) {
    for (int j = 0; j < kMapWidth; ++j) {
      const Tile* tile = &kTilemap.map[i][j];
      uint64_t type_id = tile->type;

      if (type_id == kTileOpen) continue;

      math::Vec4f color;
      switch (type_id) {
        case kTileBlock:
          color = math::Vec4f(1.f, 1.f, 1.f, 1.f);
          break;
        case kTileEngine:
          color = math::Vec4f(1.0f * sys_engine, 0.0f, 1.f * sys_engine, 1.0f);
          break;
        case kTilePower:
          color = math::Vec4f(0.0f, 0.0f, 0.75f * sys_power, 1.0f);
          break;
        case kTileMine:
          color = math::Vec4f(0.0, 0.75f * sys_mine, 0.0f, 1.0f);
          break;
        case kTileVacuum:
          color = math::Vec4f(0.33f, 0.33f, 0.33f, 1.f);
          break;
        case kTileTurret:
          color = math::Vec4f(1.f * sys_turret, 0.f, 0.f, 1.f);
          break;
      };

      rgg::RenderRectangle(math::Vec3f(TileToWorld(*tile)),
                           math::Vec3f(1.f / 2.f, 1.f / 2.f, 1.f),
                           math::Quatf(0.f, 0.f, 0.f, 1.f), color);
    }
  }

  const math::Vec2f grid2(50.f, 50.f);
  math::Rectf world2 = visible_world;
  AlignToGrid(grid2, &world2);
  rgg::RenderGrid(grid2, world2, math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f));

  const math::Vec2f grid1(25.f, 25.f);
  math::Rectf world1 = visible_world;
  AlignToGrid(grid1, &world1);
  rgg::RenderGrid(grid1, world1, math::Vec4f(0.050f, 0.215f, 0.050f, 0.45f));
}

void
PushText(const char* msg, float screen_x, float screen_y)
{
  assert(kGfx.text_count + 1 < kMaxTextCount);
  if (kGfx.text_count + 1 >= kMaxTextCount) return;
  int len = strlen(msg);
  if (len > kMaxTextSize) return;
  Text& text = kGfx.text[kGfx.text_count++];
  strcpy(text.msg, msg);
  text.screen_x = screen_x;
  text.screen_y = screen_y;
}

}  // namespace gfx
