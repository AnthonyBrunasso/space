// Singleplayer game template.
#define SINGLE_PLAYER


#include "audio/audio.cc"
#include "common/macro.h"
#include "gfx/imui.cc"
#include "math/math.cc"
#include "network/network.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "renderer/mesh.cc"
#include "search/search.cc"

#include "housefire_map.cc"

struct State {
  // Game and render updates per second
  uint64_t framerate = 60;
  // Calculated available microseconds per game_update
  uint64_t frame_target_usec;
  // Game clock state
  TscClock_t game_clock;
  // Time it took to run a frame.
  uint64_t frame_time_usec = 0;
  // Estimate of gime passed since game start.
  uint64_t game_time_usec = 0;
  // Estimated frames per second.
  float frames_per_second = 0;
  // (optional) yield unused cpu time to the system
  bool sleep_on_loop = true;
  // Number of times the game has been updated.
  uint64_t game_updates = 0;
  // Parameters window::Create will be called with.
  window::CreateInfo window_create_info;
  // Id to music.
  uint32_t music_id;
};

// Player is actual dude on screen.
struct Player {
  v2i position_map;
  v3f position_world;
  v3f dims;
};

static Player kPlayer;
static rgg::Mesh kFireMesh;

static bool kLeftClickDown = false;

static State kGameState;
static Stats kGameStats;

#define UIBUFFER_SIZE 64
static char kUIBuffer[UIBUFFER_SIZE];

static v3f kCameraDirection(.612375f, .612375f, -.5f);
static v3f kCameraPosition(-75.f, -75.f, 92.f);

static const v4f kWoodenBrown(0.521f, 0.368f, 0.258f, 1.0f);
static const v4f kWoodenBrownFire(1.0f, 0.368f, 0.258f, 1.0f);

static char kCurrentMap[64];

// If set will reset the game at the given loop
static uint64_t kResetGameAt = UINT64_MAX;
static bool kEditorMode = false;
static bool kEditMapMenu = false;
static bool kEditTileMenu = false;
static Tile* kEditTile = nullptr;

void
InitializeCamera()
{
  rgg::CameraResetAll();
  v2f viewport = window::GetWindowSize();
  rgg::Camera camera;
  camera.position = kCameraPosition;
  camera.dir = kCameraDirection;
  camera.up = v3f(0.f, 0.f, 1.f);
  camera.mode = rgg::kCameraOverhead;
  camera.viewport = viewport;
  rgg::CameraInit(camera);
}

void
InitializePlayer()
{
  kPlayer.position_map = v2i(0, 0);
  kPlayer.position_world = v3f(0.f, 0.f, 0.f);
  kPlayer.dims = v3f(10.f, 10.f, 10.f);
}

void
ResetGame()
{
  MapLoad(kCurrentMap);
  InitializePlayer();
  InitializeCamera();
  kResetGameAt = UINT64_MAX;
  rgg::GetObserver()->projection =
      rgg::DefaultPerspective(window::GetWindowSize(), 55.f);
  rgg::GetObserver()->view = rgg::CameraView();
}

void
LevelFileWalk(const char* filename)
{
  if (strncmp(filename, "asset/level", 11) == 0) {
    imui::TextOptions topt;
    topt.highlight_color = imui::kRed;
    if (imui::Text(filename, topt).clicked) {
      strcpy(kCurrentMap, filename);
      MapLoad(kCurrentMap);
      strcpy(kEditMapName, filename);
      kEditMapMenu = true;
    }
  }
}

void
EditorUI()
{
  v2f screen = window::GetWindowSize();
  {
    static bool enable_tileviewer = false;
    static v2f tileviewer_pos = v2f(screen.x - 300.f, screen.y);
    imui::PaneOptions options;
    options.width = options.max_width = 300.f;
    options.max_height = 800.f;
    imui::Begin("Map Editor", imui::kEveryoneTag, options, &tileviewer_pos,
                &enable_tileviewer);
    for (int i = 0; i < kMapX; ++i) {
      for (int j = 0; j < kMapY; ++j) {
        Tile* tile = &kMap[i][j];
        v2i tp = tile->position_map;
        imui::TextOptions toptions;
        toptions.highlight_color = imui::kRed;
        snprintf(kUIBuffer, sizeof(kUIBuffer), "Tile [%i,%i]", tp.x, tp.y);
        imui::Result ires = imui::Text(kUIBuffer, toptions);
        if (ires.highlighted) {
          rgg::DebugPushCube(
              Cubef(tile->position_world, tile->dims + v3f(2.f, 2.f, 2.f)),
                    imui::kRed);
        }
        snprintf(kUIBuffer, sizeof(kUIBuffer), "  turns %i / %i",
                 tile->turns_to_fire_max - tile->turns_to_fire,
                 tile->turns_to_fire_max);
        imui::Text(kUIBuffer);
      }
    }
    imui::End();
  }


  {
    static bool enable_admin = true;
    static v2f admin_pos = v2f(0.f, screen.y - 500.f);
    imui::PaneOptions options;
    options.width = options.max_width = 300.f;
    options.max_height = 800.f;
    imui::Begin("Admin", imui::kEveryoneTag, options, &admin_pos,
                &enable_admin);
    imui::Space(imui::kVertical, 3);
    imui::TextOptions to;
    to.highlight_color = imui::kRed;
    if (imui::Text("Reset Game", to).clicked) {
      kResetGameAt = kGameState.game_updates;
    }
    if (imui::Text("Export Map", to).clicked) {
      MapExport(kCurrentMap);
    }
    if (imui::Text("New Map", to).clicked) {
      kEditMapMenu = true;
      MapGenerateUniqueName();
    }
    static bool load_map_toggle = false;
    if (imui::Text("Load Map", to).clicked) {
      load_map_toggle = !load_map_toggle;
    }
    if (load_map_toggle) {
      imui::Indent(2);
      filesystem::WalkDirectory("asset/", LevelFileWalk);
      imui::Indent(-2);
    }
    imui::End();
  }

  {
    if (kEditMapMenu) {
      bool show_kEditMapMenu = true;
      static v2f kEditMapMenu_pos = v2f(screen.x / 2.f, screen.y / 2.f);
      static uint32_t starting_ttf = 5;
      imui::PaneOptions options;
      imui::Begin("Edit Map", imui::kEveryoneTag, options, &kEditMapMenu_pos,
                  &show_kEditMapMenu);
      static const float kWidth = 100.f;
      imui::Space(imui::kVertical, 3.f);
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Map Name");
      snprintf(kUIBuffer, sizeof(kUIBuffer), "%s", kEditMapName);
      imui::Text(kUIBuffer);
      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Map Size");
      snprintf(kUIBuffer, sizeof(kUIBuffer), "%ux%u", kMapX, kMapY);
      imui::Text(kUIBuffer);
      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Init TTF");
      snprintf(kUIBuffer, sizeof(kUIBuffer), "%u", starting_ttf);
      imui::Text(kUIBuffer);
      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Width");
      if (imui::Button(15.f, 15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
        --kMapX;
        MapInitialize(starting_ttf);
      }
      imui::Space(imui::kHorizontal, 5.f);
      if (imui::Button(15.f, 15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
        ++kMapX;
        MapInitialize(starting_ttf);
      }

      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Height");
      if (imui::Button(15.f, 15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
        --kMapY;
        MapInitialize(starting_ttf);
      }

      imui::Space(imui::kHorizontal, 5.f);
      if (imui::Button(15.f, 15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
        ++kMapY;
        MapInitialize(starting_ttf);
      }

      kMapX = CLAMP(kMapX, 0, kMapMaxX);
      kMapY = CLAMP(kMapY, 0, kMapMaxY);

      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("TTF");
      if (imui::Button(15.f, 15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
        --starting_ttf;
        MapInitialize(starting_ttf);
      }

      imui::Space(imui::kHorizontal, 5.f);
      if (imui::Button(15.f, 15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
        ++starting_ttf;
        MapInitialize(starting_ttf);
      }

      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);

      imui::TextOptions text_options;
      text_options.highlight_color = imui::kRed;

      if (imui::Text("Done", text_options).clicked) {
        kEditMapMenu = false;
        strcpy(kCurrentMap, kEditMapName);
      }

      imui::End();
    }
  }

  {
    if (kEditTileMenu && kEditTile) {
      static v2f edit_tile_pos = v2f(screen.x / 2.f, screen.y / 2.f);
      static uint32_t starting_ttf = 5;
      imui::PaneOptions options;
      imui::Begin("Edit Tile", imui::kEveryoneTag, options, &edit_tile_pos,
                  &kEditTileMenu);
      rgg::DebugPushCube(Cubef(kEditTile->position_world,
                               kEditTile->dims + v3f(2.f, 2.f, 2.f)),
                         imui::kRed);
      snprintf(kUIBuffer, sizeof(kUIBuffer), "Tile %u %u",
               kEditTile->position_map.x, kEditTile->position_map.y);
      imui::Text(kUIBuffer);
      //constexpr float width = 80.f;
      snprintf(kUIBuffer, sizeof(kUIBuffer), "TTF %u ",
               kEditTile->turns_to_fire_max);
      imui::SameLine();
      imui::Width(80.f);
      imui::Text(kUIBuffer);
      if (imui::Button(15.f, 15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
        --kEditTile->turns_to_fire_max;
        kEditTile->turns_to_fire = kEditTile->turns_to_fire_max;
      }
      imui::Space(imui::kHorizontal, 5.f);
      if (imui::Button(15.f, 15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
        ++kEditTile->turns_to_fire_max;
        kEditTile->turns_to_fire = kEditTile->turns_to_fire_max;
      }
      imui::NewLine();
      imui::SameLine();
      imui::Width(80.f);
      imui::Text("Preset");
      imui::TextOptions to;
      to.color = imui::kWhite;
      to.highlight_color = imui::kRed;
      if (imui::Text("1 ", to).clicked) {
        kEditTile->turns_to_fire_max = 1;
        kEditTile->turns_to_fire = kEditTile->turns_to_fire_max;
      }
      if (imui::Text("5 ", to).clicked) {
        kEditTile->turns_to_fire_max = 5;
        kEditTile->turns_to_fire = kEditTile->turns_to_fire_max;
      }
      if (imui::Text("10 ", to).clicked) {
        kEditTile->turns_to_fire_max = 10;
        kEditTile->turns_to_fire = kEditTile->turns_to_fire_max;
      }
      if (imui::Text("15 ", to).clicked) {
        kEditTile->turns_to_fire_max = 15;
        kEditTile->turns_to_fire = kEditTile->turns_to_fire_max;
      }
      if (imui::Text("20 ", to).clicked) {
        kEditTile->turns_to_fire_max = 20;
        kEditTile->turns_to_fire = kEditTile->turns_to_fire_max;
      }
      imui::NewLine();
      imui::SameLine();
      bool is_destination = FLAGGED(kEditTile->flags, kTileDestination);
      imui::Checkbox(17.f, 17.f, &is_destination);
      imui::Space(imui::kHorizontal, 2.f);
      imui::Text("Destination");
      if (is_destination) {
        SBIT(kEditTile->flags, kTileDestination);
      } else {
        CBIT(kEditTile->flags, kTileDestination);
      }
      imui::End();
    }
  }

  {
    static bool enable_debug = true;
    static v2f diagnostics_pos(3.f, screen.y);
    static float right_align = 130.f;
    imui::PaneOptions options;
    options.max_width = 350.f;
    imui::Begin("Diagnostics", imui::kEveryoneTag, options, &diagnostics_pos,
                &enable_debug);
    imui::TextOptions debug_options;
    debug_options.color = imui::kWhite;
    debug_options.highlight_color = imui::kRed;
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Frame Time");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%04.02fus [%02.02f%%]",
             StatsMean(&kGameStats), 100.f * StatsUnbiasedRsDev(&kGameStats));
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Game Time");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%04.02fs",
             (double)kGameState.game_time_usec / 1e6);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("FPS");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%04.02ff/s",
             (double)kGameState.game_updates /
                 ((double)kGameState.game_time_usec / 1e6));
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Window Size");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%.0fx%.0f", screen.x, screen.y);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Camera Pos");
    v3f p = rgg::CameraPosition();
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%.3f %.3f %.3f", p.x, p.y, p.z);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Camera Dir");
    v3f d = rgg::CameraDirection();
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%.3f %.3f %.3f", d.x, d.y, d.z);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::End();
  }

#if UIDEBUG
  {
    static bool enable_debug = false;
    static v2f ui_pos(300.f, screen.y);
    imui::DebugPane("UI Debug", imui::kEveryoneTag, &ui_pos, &enable_debug);
  }
#endif
}

bool
GraphicsInitialize(const window::CreateInfo& window_create_info)
{
  int window_result = window::Create("Space", window_create_info);
  if (!rgg::Initialize()) return false;
  if (!rgg::LoadOBJ("asset/fire.obj", &kFireMesh)) {
    printf("Unable to load fire.obj\n");
    return false;
  } 
  return true;
}

bool
CanMoveTo(const v2i& target, v2i* possible_move, uint32_t possible_move_count)
{
  for (int i = 0; i < possible_move_count; ++i) {
    if (possible_move[i] == target) return true;
  }
  return false;
}

bool
TileIsBlocked(const v2i& from)
{
  return kMap[from.x][from.y].turns_to_fire == 0;
}


v2i
TileNeighbor(const v2i& from, uint32_t i)
{
  static const v2i kNeighbor[kMaxTileNeighbor] = {
      v2i(-1, 0), v2i(1, 0),  v2i(0, 1),  v2i(0, -1),
      v2i(1, 1),  v2i(-1, 1), v2i(1, -1), v2i(-1, -1)};
  return from + kNeighbor[i % kMaxTileNeighbor];
}

search::BfsIterator
SetupBfsIterator(const v2i& from, uint32_t max_depth = UINT32_MAX)
{
  search::BfsIterator itr = {};
  itr.blocked_callback = TileIsBlocked;
  itr.neighbor_callback = TileNeighbor;
  itr.max_neighbor = kMaxTileNeighbor;
  itr.current = from;
  itr.map_size = v2i(kMapX, kMapY);
  itr.max_depth = max_depth;
  SBIT(itr.flags, search::kAvoidBlockedDiagnol);
  return itr;
}

void
DebugRenderOnTile(const v2i& pos, const v4f& color)
{
  Tile* t = &kMap[pos.x][pos.y];
  rgg::DebugPushCube(Cubef(t->position_world, t->dims), color);
  // The OpenGL gods need to teach me how to make thick lines.
  rgg::DebugPushCube(Cubef(t->position_world, t->dims + v3f(.05f, .05f, .05f)), color);
  rgg::DebugPushCube(Cubef(t->position_world, t->dims + v3f(.1f, .1f, .1f)), color);
  rgg::DebugPushCube(Cubef(t->position_world, t->dims + v3f(.15f, .15f, .15f)), color);
  rgg::DebugPushCube(Cubef(t->position_world, t->dims + v3f(.2f, .2f, .2f)), color);
}

Tile*
TileHover(const v2f& cursor)
{
  v3f cray = rgg::CameraRayFromMouse(cursor);
  float d = 0;
  v3f n(0.f, 0.f, 1.f);
  float t = -(math::Dot(rgg::CameraPosition(), n) + d) / math::Dot(cray, n);
  v3f res = rgg::CameraPosition() + cray * t;
  //rgg::DebugPushSphere(res, 2.f, v4f(0.f, 1.f, 0.f, 0.8f));
  for (int i = 0; i < kMapX; ++i) {
    for (int j = 0; j < kMapY; ++j) {
      Tile* tile = &kMap[i][j];
      if (math::PointInRect(
            // TODO: Investigate this - implies render vs intersection mismatch.
            // One likely calculating from middle and other bottom left.
            res.xy() + v2f(kTileWidth / 2.f, kTileHeight / 2.f),
            Rectf(tile->position_world.xy(), tile->dims.xy()))) {
        return tile;
      }
    }
  }
  return nullptr;
}

void
ProcessWorldTurn()
{
  for (int i = 0; i < kMapX; ++i) {
    for (int j = 0; j < kMapY; ++j) {
      if (kMap[i][j].turns_to_fire) kMap[i][j].turns_to_fire--;
      if (FLAGGED(kMap[i][j].flags, kTileDestination) &&
          kPlayer.position_map == v2i(i, j)) {
        MapSetNextLevel(kCurrentMap);
        kResetGameAt = kGameState.game_updates + 100;
      }
      if (!kMap[i][j].turns_to_fire) {
        if (kPlayer.position_map == v2i(i, j)) {
          // Player died - reset the game in N game updates so player
          // can see they are standing in fire.
          kResetGameAt = kGameState.game_updates + 100;
        }
      }
    }
  }
}

void
GameUpdate()
{
  v2i possible_move[16];
  uint32_t possible_move_count = 0;
  v2f cursor = window::GetCursorPosition();
  search::BfsIterator bfs_itr =
        SetupBfsIterator(kPlayer.position_map, 1);
  if (search::BfsStart(&bfs_itr)) {
    while (search::BfsNext(&bfs_itr)) {
      DebugRenderOnTile(bfs_itr.current, v4f(0.f, 1.f, 1.f, 1.f));
      assert(possible_move_count < 16);
      possible_move[possible_move_count++] = bfs_itr.current;
    }
  }
  Tile* tile = TileHover(cursor);
  if (tile) {
    static float depth = 1.f;
    v4f color = v4f(0.f, .99f, .33f, 1.f);
    if (TileIsBlocked(tile->position_map)) {
      color = v4f(0.99f, 0.f, .33f, 1.f);
    } else if (!CanMoveTo(
        tile->position_map, possible_move, possible_move_count)) {
      search::BfsIterator bfs_itr = SetupBfsIterator(kPlayer.position_map);
      search::Path* path = search::BfsPathTo(&bfs_itr, tile->position_map);
      if (path && path->size > 0) {
        for (int i = 0; i < path->size; ++i) {
          Tile* t = &kMap[path->queue[i].x][path->queue[i].y];
          rgg::DebugPushCube(
              Cubef(t->position_world.x, t->position_world.y, 0.f,
                    t->dims.x, t->dims.y, .1f),
                    v4f(0.5f, 0.5f, 0.5f, 1.f));
        }
      }
      color = v4f(0.3f, .3f, .3f, 0.f);
    }
    
    rgg::DebugPushCube(
        Cubef(tile->position_world.x, tile->position_world.y, 0.f,
              tile->dims.x, tile->dims.y, .1f), color);

    v2i tp = tile->position_map;
    if (kLeftClickDown && !TileIsBlocked(tp)) {
      bool can_move = CanMoveTo(tp, possible_move, possible_move_count);
      if (!can_move) {
        printf("Unable to move to %i,%i\n", tp.x, tp.y);
      } else {
        v2f delta =
          tile->position_world.xy() - kPlayer.position_world.xy();
        rgg::CameraMove(delta);
        kPlayer.position_world.xy() = tile->position_world.xy();
        kPlayer.position_map = tp;
        ProcessWorldTurn();
      }
    }
  }
}

void
EditUpdate()
{
  v2f cursor = window::GetCursorPosition();
  EditorUI();
  Tile* tile = TileHover(cursor);
  if (tile && !imui::MouseInUI(imui::kEveryoneTag)) {
    rgg::DebugPushCube(
        Cubef(tile->position_world.x, tile->position_world.y, 0.f,
              tile->dims.x, tile->dims.y, .1f), v4f(0.f, 1.f, 0.f, 1.f));
    if (kLeftClickDown) {
      kEditTile = tile;
      kEditTileMenu = true;
    }
  }
}

bool
AdjacentTileBlocked(Tile* t)
{
  if (!t) return false;
  for (int i = 0; i < kMaxTileNeighbor; ++i) {
    v2i pn = TileNeighbor(t->position_map, i);
    if (!search::IsInMap(pn, v2i(kMapX, kMapY))) continue;
    Tile* nt = &kMap[pn.x][pn.y];
    if (!nt->turns_to_fire) return true;
  }
  return false;
}

void
Render()
{
  static int dumb = 0;
  ++dumb;
  for (int i = 0; i < kMapX; ++i) {
    for (int j = 0; j < kMapY; ++j) {
      Tile* t = &kMap[i][j];
      if (t->turns_to_fire) {
        float lerpt = 0.f;
        if (AdjacentTileBlocked(t) || t->turns_to_fire == 1) {
          lerpt = 1.f - (float)t->turns_to_fire / (float)t->turns_to_fire_max;
        }
        if (FLAGGED(t->flags, kTileDestination)) {
          rgg::RenderCube(Cubef(t->position_world, t->dims),
                          v4f(.33f, .77f, .11f, 1.f));
        } else {
          rgg::RenderCube(
              Cubef(t->position_world, t->dims),
              math::Lerp(kWoodenBrown, kWoodenBrownFire, lerpt));
        }
      } else {
        static float zd[kMapMaxX][kMapMaxY] = {};  // LOL
        static const uint32_t kDumbMod = 10;
        uint32_t mdumb = dumb % kDumbMod;
        if (mdumb == 0) {
          zd[i][j] = math::ScaleRange((float)rand() / RAND_MAX, 0.f, 1.f, -1.f, 1.f);
          //printf("%.2f\n", zd);
        }
        v3f scale = v3f(7.f, 7.f, 7.f);
        v3f dumb_scale = v3f(7.f, 7.f, 7.f + zd[i][j]);
        float lerp_dumb = (float)mdumb / kDumbMod;
        v3f lerped_scale = math::Lerp(scale, dumb_scale, lerp_dumb);
        rgg::RenderMesh(kFireMesh, t->position_world + v3f(0.f, 0.f, 3.f),
                        lerped_scale,
                        Quatf(280.f, v3f(1.f, 0.f, 0.f)));
        rgg::RenderCube(Cubef(t->position_world, t->dims), kWoodenBrownFire);
      }
    }
  }

  rgg::RenderCube(Cubef(kPlayer.position_world + v3f(0.f, 0.f, kTileDepth / 2.f),
                        kPlayer.dims), v4f(.3f, .3f, 1.f, .8f));


  rgg::DebugRenderPrimitives();

  imui::Render(imui::kEveryoneTag);
}

int
main(int argc, char** argv)
{
#if __APPLE__
  kGameState.window_create_info.window_width = 1280;
  kGameState.window_create_info.window_height = 720;
#endif
  if (!GraphicsInitialize(kGameState.window_create_info)) {
    return 1;
  }

  if (!audio::Initialize()) {
    printf("Unable to initialize audio system.\n");
    return 1;
  }

  strcpy(kCurrentMap, "asset/level_0.map");
  MapLoad(kCurrentMap);

  InitializePlayer();
  InitializeCamera();

  rgg::GetObserver()->projection =
      rgg::DefaultPerspective(window::GetWindowSize(), 55.f);
  rgg::GetObserver()->view = rgg::CameraView();

  // main thread affinity set to core 0
  if (platform::thread_affinity_count() > 1) {
    platform::thread_affinity_usecore(0);
    printf("Game thread may run on %d cores\n",
           platform::thread_affinity_count());
  }

  // Reset State
  StatsInit(&kGameStats);
  kGameState.game_updates = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  printf("Client target usec %lu\n", kGameState.frame_target_usec);

  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  clock_init(kGameState.frame_target_usec, &kGameState.game_clock);
  printf("median_tsc_per_usec %lu\n", median_tsc_per_usec);

  while (1) {
    imui::ResetTag(imui::kEveryoneTag);
    rgg::DebugReset();

    if (window::ShouldClose()) break;

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);
    bool is_mouse_in_ui = imui::MouseInUI(imui::kEveryoneTag);

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch(event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 27 /* ESC */: {
              exit(1);
            } break;
            case -64:  // TODO: Unexpected windows code for '`'.
            case '`': {
              kEditorMode = !kEditorMode;
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
          if (!is_mouse_in_ui) kLeftClickDown = true;
        } break;
        case MOUSE_UP: {
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
          if (!is_mouse_in_ui) kLeftClickDown = false;
        } break;
        case MOUSE_WHEEL: {
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
        } break;
      }
      
      if (!is_mouse_in_ui) {
        rgg::CameraUpdateEvent(event);
        rgg::GetObserver()->view = rgg::CameraView();
        // This is really the lightl position...
        v3f xyforward = math::Normalize(rgg::CameraDirection().xy());
        //rgg::GetObserver()->position = rgg::CameraPosition() + xyforward * 500.f;
        rgg::GetObserver()->position = rgg::CameraPosition();
      }
    }

    // If game not meant to reset...
    if (!kEditorMode && kResetGameAt == UINT64_MAX) {
      GameUpdate();
    } else if (kEditorMode) {
      EditUpdate();
    }

    // Left clicks should only be processed for the single frame.
    kLeftClickDown = false;


    if (kGameState.game_updates >= kResetGameAt) {
      ResetGame();
    }
    
    Render();
        
    const uint64_t elapsed_usec = clock_delta_usec(&kGameState.game_clock);
    kGameState.frame_time_usec = elapsed_usec;
    StatsAdd(elapsed_usec, &kGameStats);
    kGameState.game_time_usec += elapsed_usec;

    window::SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    uint64_t sleep_usec = 0;
    uint64_t sleep_count = kGameState.sleep_on_loop;
    while (!clock_sync(&kGameState.game_clock, &sleep_usec)) {
      while (sleep_count) {
        --sleep_count;
        platform::sleep_usec(sleep_usec);
        kGameState.game_time_usec += sleep_usec;
      }
    }

    kGameState.game_updates++;
  }

  return 0;
}
