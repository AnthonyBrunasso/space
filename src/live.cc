// Singleplayer game template.
#define SINGLE_PLAYER

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "math/math.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "renderer/imui.cc"
#include "animation/fsm.cc"

#define WIN_ATTACH_DEBUGGER 0
#define DEBUG_PHYSICS 0
#define DEBUG_UI 0
#define PLATFORMER_CAMERA 1

static b8 kRenderCharacterAabb = false;
static b8 kRenderGrid = false;
static b8 kRenderGridFill = false;
static b8 kDebugImui = false;

struct State {
  // Game and render updates per second
  u64 framerate = 60;
  // Calculated available microseconds per game_update
  u64 frame_target_usec;
  // Estimate of gime passed since game start.
  u64 game_time_usec = 0;
  // Estimated frames per second.
  r32 frames_per_second = 0;
  // (optional) yield unused cpu time to the system
  b8 sleep_on_loop = true;
  // Number of times the game has been updated.
  u64 game_updates = 0;
  // Parameters window::Create will be called with.
  window::CreateInfo window_create_info;
  // Id to music.
  u32 music_id;
  // Game clock.
  platform::Clock game_clock;
};


static State kGameState;
static Stats kGameStats;

namespace std {

template <>
struct hash<v2i>
{
  std::size_t
  operator()(const v2i& grid) const
  {
    // Arbitrarily large prime numbers
    const size_t h1 = 0x8da6b343;
    const size_t h2 = 0xd8163841;
    return grid.x * h1 + grid.y * h2;
  }
};

}

#include "live/components.cc"
#include "live/constants.cc"
#include "live/sim.cc"

static char kUIBuffer[64];

enum DebugUIState {
  kDiagnosticsViewer,
  kEntityViewer,
  kDebugViewer,
};

void
SetFramerate(u64 fr)
{
  kGameState.framerate = fr;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
}

void
DebugUIRenderDiagnostics()
{
  v2f screen = window::GetWindowSize();
  static r32 right_align = 130.f;
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
           (r64)kGameState.game_time_usec / 1e6);
  imui::Text(kUIBuffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("AVG FPS");
  snprintf(kUIBuffer, sizeof(kUIBuffer), "%04.02ff/s",
           (r64)kGameState.game_updates /
               ((r64)kGameState.game_time_usec / 1e6));
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
  v3f cpos = rgg::CameraPosition();
  snprintf(kUIBuffer, sizeof(kUIBuffer), "(%.0f, %.0f, %.0f)", cpos.x, cpos.y, cpos.z);
  imui::Text(kUIBuffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Cursor World");
  v2f wpos = rgg::CameraRayFromMouseToWorld(window::GetCursorPosition(), 1.f).xy();
  snprintf(kUIBuffer, sizeof(kUIBuffer), "(%.0f, %.0f)", wpos.x, wpos.y);
  imui::Text(kUIBuffer);
  imui::NewLine();
  v2i gpos;
  if (live::GridXYFromPos(wpos, &gpos)) {
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Cursor Grid");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "(%i, %i)", gpos.x, gpos.y);
    imui::Text(kUIBuffer);
    imui::NewLine();
  }
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Game Speed");
  if (imui::Text("120 ", debug_options).clicked) {
    SetFramerate(120);
  }
  if (imui::Text("60 ", debug_options).clicked) {
    SetFramerate(60);
  }
  if (imui::Text("30 ", debug_options).clicked) {
    SetFramerate(30);
  }
  if (imui::Text("10 ", debug_options).clicked) {
    SetFramerate(10);
  }
  if (imui::Text("5 ", debug_options).clicked) {
    SetFramerate(5);
  }
  imui::NewLine();
}

void
DebugUIRenderEntity()
{
  static r32 right_align = 130.f;
  // TODO: Current grid
  live::Grid* grid = live::GridGet(1);
  v2f wpos = rgg::CameraRayFromMouseToWorld(window::GetCursorPosition(), 1.f).xy();
  v2i gpos;
  if (live::GridXYFromPos(wpos, &gpos)) {
    live::Cell* cell = grid->Get(gpos);
    assert(cell != nullptr);
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Cursor Grid");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "(%i, %i)", gpos.x, gpos.y);
    imui::Text(kUIBuffer);
    imui::NewLine();
    for (u32 entity_id : cell->entity_ids) {
      ecs::Entity* entity = ecs::FindEntity(entity_id);
      assert(entity != nullptr);
      imui::SameLine();
      snprintf(kUIBuffer, sizeof(kUIBuffer), "Entity(%u)", entity_id);
      imui::Text(kUIBuffer);
      imui::NewLine();
      for (u64 i = 0; i < ecs::kComponentCount; ++i) {
        ecs::TypeId type_id = (ecs::TypeId)i;
        if (entity->Has(type_id)) {
          snprintf(kUIBuffer, sizeof(kUIBuffer), "  %s", ecs::TypeName(type_id));
          imui::Text(kUIBuffer);
        }
      }
    }
  }
}

void
DebugUIRenderDebug()
{
  static r32 right_align = 160.f;
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Render AABB");
  imui::Checkbox(16.f, 16.f, &kRenderCharacterAabb);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Render Grid");
  imui::Checkbox(16.f, 16.f, &kRenderGrid);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Render Grid Fill");
  imui::Checkbox(16.f, 16.f, &kRenderGridFill);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Debug IMUI");
  imui::Checkbox(16.f, 16.f, &kDebugImui);
  imui::NewLine();
}

void
DebugUI()
{
  v2f screen = window::GetWindowSize();
  {
    static b8 enable_debug = false;
    static v2f diagnostics_pos(3.f, screen.y);
    static DebugUIState debug_ui_state = kDiagnosticsViewer;
    static r32 right_align = 125.f;
    static v4f unused_color = v4f(.8f, .8f, .8f, 1.f);
    imui::PaneOptions options;
    options.max_width = 315.f;
    imui::Begin("Debug", imui::kEveryoneTag, options, &diagnostics_pos,
                &enable_debug);
    imui::TextOptions debug_options;
    debug_options.highlight_color = imui::kRed;
    imui::SameLine();
    imui::Width(right_align);
    debug_options.color = debug_ui_state == kDiagnosticsViewer ? imui::kRed : unused_color;
    if (imui::Text("Diag", debug_options).clicked) {
      debug_ui_state = kDiagnosticsViewer;
    }
    imui::Width(right_align);
    debug_options.color = debug_ui_state == kEntityViewer ? imui::kRed :  unused_color;
    if (imui::Text("Entity", debug_options).clicked) {
      debug_ui_state = kEntityViewer;
    }
    debug_options.color = debug_ui_state == kDebugViewer ? imui::kRed :  unused_color;
    if (imui::Text("Debug", debug_options).clicked) {
      debug_ui_state = kDebugViewer;
    }
    imui::NewLine();
    imui::HorizontalLine(v4f(1.f, 1.f, 1.f, .4f));
    imui::Space(imui::kVertical, 5);
    switch (debug_ui_state) {
      case kDiagnosticsViewer:
        DebugUIRenderDiagnostics();
        break;
      case kEntityViewer:
        DebugUIRenderEntity();
        break;
      case kDebugViewer:
        DebugUIRenderDebug();
        break;
    }
    imui::End();
  }
}

void
GameInitialize(const v2f& dims)
{
  rgg::GetObserver()->projection =
    math::Ortho(dims.x, 0.f, dims.y, 0.f, -1.f, 1.f);

  rgg::Camera camera;
  camera.position = v3f(950.f, 600.f, 0.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.viewport = dims;
  camera.mode = rgg::kCameraOverhead;
  rgg::CameraInit(camera);

  // I don't think we need depth testing for a 2d game?
  //glDisable(GL_DEPTH_TEST);
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  
  
  live::SimInitialize();
}

bool
GameUpdate()
{
  rgg::CameraUpdate();
  rgg::GetObserver()->view = rgg::CameraView();
  //Print4x4Matrix(rgg::GetObserver()->view);

  live::SimUpdate();

  return true;
}

void
GameRender(v2f dims)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(.1f, .1f, .13f, 1.f);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_ALWAYS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  


  DebugUI();
  //live::InteractionRenderEntityViewer();
  live::InteractionRenderOrderOptions();
  live::InteractionRenderResourceCounts();

  if (kRenderGridFill) {
    live::Grid* grid = live::GridGet(1);
    for (live::Cell& cell : grid->storage) {
      if (!cell.entity_ids.empty()) {
        rgg::RenderRectangle(cell.rect(), v4f(.2f, .2f, .2f, .8f));
      }
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kZoneComponent);
    while (itr.Next()) {
      PhysicsComponent* physics = itr.c.physics;
      rgg::RenderRectangle(physics->rect(), v4f(0.1f, 0.1f, 0.4f, 0.8f));
    }
  }
  
  {
    ECS_ITR3(itr, kPhysicsComponent, kHarvestComponent, kResourceComponent);
    while (itr.Next()) {
      PhysicsComponent* physics = itr.c.physics;
      ResourceComponent* resource = itr.c.resource;
      switch (resource->resource_type) {
        case kLumber:
          rgg::RenderRectangle(physics->rect(), v4f(0.f, 1.f, 0.f, 1.f));
          break;
        case kStone:
          rgg::RenderRectangle(physics->rect(), v4f(.5f, .5f, .5f, 1.f));
          break;
        case kResourceTypeCount:
        default:
          assert(!"Can't render resource type");
      }
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kBuildComponent);
    while (itr.Next()) {
      PhysicsComponent* physics = itr.c.physics;
      BuildComponent* build = itr.c.build;
      switch (build->structure_type) {
        case kWall:
          rgg::RenderLineRectangle(physics->rect(), v4f(1.f, 1.f, 1.f, 1.f));
          break;
        case kStructureTypeCount:
        default:
          assert(!"Can't render build component");
      }
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kStructureComponent);
    while (itr.Next()) {
      PhysicsComponent* physics = itr.c.physics;
      StructureComponent* structure = itr.c.structure;
      switch (structure->structure_type) {
        case kWall:
          rgg::RenderRectangle(physics->rect(), v4f(.64f, .45f, .28f, 1.f));
          break;
        case kStructureTypeCount:
        default:
          assert(!"Can't render structure component");
      }
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kCharacterComponent);
    while (itr.Next()) {
      PhysicsComponent* character = itr.c.physics;
      r32 half_width = character->rect().width / 2.f;

      /*
      std::vector<v2i> grids = live::GridGetIntersectingCellPos(character);
      for (v2i grid : grids) {
        v2f grid_pos = live::GridPosFromXY(grid);
        rgg::RenderRectangle(Rectf(grid_pos, live::CellDims()), v4f(.2f, .6f, .2f, .8f));
      }*/

      rgg::RenderCircle(character->pos + v2f(half_width, half_width),
                        character->rect().width / 2.f, v4f(1.f, 0.f, 0.f, 1.f));

      if (kRenderCharacterAabb) {
        rgg::RenderLineRectangle(character->rect(), v4f(1.f, 0.f, 0.f, 1.f));
      }
      
      //v2f grid_pos;
      //if (live::GridClampPos(character->pos, &grid_pos)) {
      //  rgg::RenderRectangle(Rectf(grid_pos, live::CellDims()), v4f(.2f, .2f, .2f, .8f));
      //}
    }
  }

  {
    ECS_ITR2(itr, kPhysicsComponent, kResourceComponent);
    while (itr.Next()) {
      // The resource should not be rendered if this is a tree for instance.
      if (GetHarvestComponent(itr.e)) continue;
      PhysicsComponent* physics = itr.c.physics;
      ResourceComponent* resource = itr.c.resource;
      Rectf rect = physics->rect();
      switch (resource->resource_type) {
        case kLumber:
          rgg::RenderTriangle(rect.Center(), rect.width / 2.f, v4f(.64f, .45f, .28f, 1.f));
          break;
        case kStone:
          rgg::RenderTriangle(rect.Center(), rect.width / 2.f, v4f(.5f, .5f, .5f, 1.f));
          break;
        case kResourceTypeCount:
        default:
          assert(!"Can't render resource type");
      }
    }
  }

  if (kRenderGrid) {
    // TODO: Implement an active grid which is the current view I think.
    live::Grid* grid = live::GridGet(1);
    r32 grid_width = grid->width * live::kCellWidth;
    r32 grid_height = grid->height * live::kCellHeight;
    for (s32 x = 0; x < grid->width; ++x) {
      v2f start(x * live::kCellWidth, 0);
      v2f end(x * live::kCellWidth, grid_height);
      rgg::RenderLine(start, end, v4f(1.f, 1.f, 1.f, .15f));
    }
    for (s32 y = 0; y < grid->height; ++y) {
      v2f start(0, y * live::kCellHeight);
      v2f end(grid_width, y * live::kCellHeight);
      rgg::RenderLine(start, end, v4f(1.f, 1.f, 1.f, .15f));
    }
  }

  live::InteractionRender();

  rgg::DebugRenderWorldPrimitives();

  rgg::DebugRenderUIPrimitives();

  if (kDebugImui) {
    static v2f debug_imui(500.f, 500.f);
    static b8 show = true;
    imui::DebugPane("Debug IMUI", imui::kEveryoneTag, &debug_imui, &show);
  }
  imui::Render(imui::kEveryoneTag);

  window::SwapBuffers();
}

void
ProcessPlatformEvent(const PlatformEvent& event) {
  rgg::CameraUpdateEvent(event);
  switch(event.type) {
    case KEY_DOWN: {
      switch (event.key) {
        case KEY_ESC: {
          exit(1);
        } break;
        case KEY_ARROW_UP: {
          rgg::CameraMove(v2f(0.f, live::kCameraSpeed));
        } break;
        case KEY_ARROW_RIGHT: {
          rgg::CameraMove(v2f(live::kCameraSpeed, 0.f));
        } break;
        case KEY_ARROW_DOWN: {
          rgg::CameraMove(v2f(0.f, -live::kCameraSpeed));
        } break;
        case KEY_ARROW_LEFT: {
          rgg::CameraMove(v2f(-live::kCameraSpeed, 0.f));
        } break;
      }
    } break;
    case MOUSE_DOWN: {
      if (event.button == BUTTON_LEFT) {
        imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
      }
    } break;
    case MOUSE_UP: {
      imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
    } break;
    default: break;
  }

  live::InteractionProcessPlatformEvent(event);
}

s32
main(s32 argc, char** argv)
{
#ifdef _WIN32
#if WIN_ATTACH_DEBUGGER
  while (!IsDebuggerPresent()) {};
#endif
#endif
  if (!memory::Initialize(MiB(64))) {
    return 1;
  }

  kGameState.window_create_info.window_width = live::kScreenWidth;
  kGameState.window_create_info.window_height = live::kScreenHeight;
  if (!window::Create("Game", kGameState.window_create_info)) {
    return 1;
  }

  if (!rgg::Initialize()) {
    return 1;
  }

  const v2f dims = window::GetWindowSize();
  GameInitialize(dims);
  
  // main thread affinity set to core 0
  if (platform::thread_affinity_count() > 1) {
    platform::thread_affinity_usecore(0);
    printf("Game thread may run on %d cores\n",
           platform::thread_affinity_count());
  }

  // Reset State
  StatsInit(&kGameStats);
  kGameState.game_updates = 0;
  SetFramerate(kGameState.framerate);
  printf("Client target usec %lu\n", kGameState.frame_target_usec);

  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();

  while (1) {
    platform::ClockStart(&kGameState.game_clock);

    imui::ResetTag(imui::kEveryoneTag);
    rgg::DebugReset();

    if (window::ShouldClose()) break;

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      ProcessPlatformEvent(event);
    }

    GameUpdate();
    GameRender(dims);  

    const u64 elapsed_usec = platform::ClockEnd(&kGameState.game_clock);
    StatsAdd(elapsed_usec, &kGameStats);

    if (kGameState.frame_target_usec > elapsed_usec) {
      u64 wait_usec = kGameState.frame_target_usec - elapsed_usec;
      platform::Clock wait_clock;
      platform::ClockStart(&wait_clock);
      while (platform::ClockEnd(&wait_clock) < wait_usec) {}
    }

    kGameState.game_time_usec += platform::ClockEnd(&kGameState.game_clock);
    kGameState.game_updates++;
  }

  return 0;
}
