// Singleplayer game template.
#define SINGLE_PLAYER

#include <cassert>
#include <chrono>
#include <optional>
#include <thread>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "math/math.cc"

#include "4xsim.grpc.pb.h"
#include "renderer/camera.cc"
#include "renderer/renderer.cc"
#include "renderer/imui.cc"
#include "4x/sim.cc"
#include "4x/server.cc"

ABSL_FLAG(std::string, address, "127.0.0.1:3878",
          "If unspecified will spawn the server locally - otherwise this is "
          "assumed a multiplayer game.");

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
};

static State kGameState;
static Stats kGameStats;

static v2i kHighlighted;

static std::thread* kServerThread;

#define UIBUFFER_SIZE 64
static char kUIBuffer[UIBUFFER_SIZE];

constexpr u32 kHexMapSize = 10;

void
MaybeStartGameServer()
{
  if (kServerThread) {
    printf("Game server already spawned.\n");
    return;
  }

  std::string address = absl::GetFlag(FLAGS_address);
  if (address.find("127.0.0.1") != std::string::npos) {
    kServerThread = new std::thread(fourx::RunServer, address);
    // Give her a moment to boot up.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

void
DebugUI(fourx::HexMap& hex_map)
{
  v2f screen = window::GetWindowSize();
  {
    static b8 enable_debug = true;
    static v2f diagnostics_pos(3.f, screen.y);
    static r32 right_align = 130.f;
    imui::PaneOptions options;
    options.max_width = 415.f;
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
             (r64)kGameState.game_time_usec / 1e6);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("FPS");
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
    imui::Text("Grid Location");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "[%i %i]",
             kHighlighted.x, kHighlighted.y);
    imui::Text(kUIBuffer);
    imui::NewLine();
    imui::SameLine();
    imui::Width(right_align);
    imui::End();
  }
}

void
LogPanel()
{
  v2f screen = window::GetWindowSize();
  imui::PaneOptions pane_options;
  pane_options.width = pane_options.max_width = 450.f;
  pane_options.height = pane_options.max_height = 160.f;
  pane_options.enable_console_mode = true;
  imui::TextOptions text_options;
  static v2f pos(screen.x - pane_options.width, pane_options.height);
  static b8 show = true;
  imui::Begin("Console Log", imui::kEveryoneTag, pane_options, &pos, &show);
  //for (int i = 0, imax = LogCount(); i < imax; ++i) {
  //  const char* log_msg = ReadLog(i);
  //  if (!log_msg) continue;
  //  imui::Text(log_msg, text_options);
  //}
  imui::End();

}

void
RenderHexGridCoord(fourx::HexMap& hex_map, v2i cord)
{
  v2f world = math::HexAxialToWorld(cord, 5.f);
  rgg::RenderHexagon(v3f(world, -50.f), v3f(1.f, 1.f, 1.f),
                     v4f(.2f, .2f, .2f, 1.f));
  rgg::RenderLineHexagon(v3f(world, -49.9f), 5.f, rgg::kWhite);
  fourx::HexTile* tile = hex_map.tile(cord);
  if (!tile) return;
  if (tile->blocked) {
    rgg::RenderCube(Cubef(v3f(world, -50.f), 2.5f, 2.5f, 2.5f), rgg::kRed);
  }
}

void
UpdateGame(const v2f& cursor, b8 mouse_down, v2f& mouse_start, rgg::Camera& camera)
{
  if (mouse_down) {
    v2f diff = cursor - mouse_start;
    if (fabs(diff.x) > 0.f || fabs(diff.y) > 0.f) {
      camera.PitchYawDelta(diff.yx() * 0.1f);
      mouse_start = cursor;
    }
  }
}

void
RenderGame(const v2f& cursor, rgg::Camera& camera, fourx::HexMap& hex_map)
{
  rgg::GetObserver()->view = camera.View();

  v3f p = camera.RayFromScreenToWorld(cursor, window::GetWindowSize(), 50.f);
  v2i picked = HexWorldToAxial(p.xy(), 5.f);
  kHighlighted = picked;

  for (const auto& t : hex_map.tiles()) {
    RenderHexGridCoord(hex_map, t.grid_pos);
  }

  v2f world = math::HexAxialToWorld(picked, 5.f);
  rgg::RenderLineHexagon(v3f(world, -49.5f), 5.f, rgg::kRed);
  rgg::RenderLineHexagon(v3f(world, -49.0f), 5.f, rgg::kRed);
  rgg::RenderLineHexagon(v3f(world, -48.5f), 5.f, rgg::kRed);

  DebugUI(hex_map);

  rgg::DebugRenderPrimitives();
  imui::Render(imui::kEveryoneTag);
  window::SwapBuffers();
}

s32
main(s32 argc, char** argv)
{
  MaybeStartGameServer();

  platform::Clock game_clock;

  if (!memory::Initialize(MiB(64))) {
    return 1;
  }

  if (!window::Create("Game", kGameState.window_create_info)) {
    return 1;
  }

  if (!rgg::Initialize()) {
    return 1;
  }

  // Reset State
  StatsInit(&kGameStats);
  kGameState.game_updates = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  printf("Client target usec %lu\n", kGameState.frame_target_usec);

  rgg::Camera camera(v3f(0.f, 0.f, 0.f), v3f(0.f, 1.f, 0.f));

  rgg::GetObserver()->projection =
      rgg::DefaultPerspective(window::GetWindowSize());

  fourx::HexMap hex_map(kHexMapSize);

  bool mouse_down = false;
  v2f mouse_start;

  while (1) {
    platform::ClockStart(&game_clock);

    imui::ResetTag(imui::kEveryoneTag);
    rgg::DebugReset();

    if (window::ShouldClose()) break;

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch(event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 27 /* ESC */: {
              exit(1);
            } break;
            case 'w': {
              camera.Translate(v3f(0.f, 1.f, 0.f), v3f(1.f, 0.f, 0.f),
                               v3f(0.f, 1.f, 0.f), v3f(0.f, 0.f, -1.f));
            } break;
            case 'a': {
              camera.Translate(v3f(-1.f, 0.f, 0.f), v3f(1.f, 0.f, 0.f),
                               v3f(0.f, 1.f, 0.f), v3f(0.f, 0.f, -1.f));
            } break;
            case 's': {
              camera.Translate(v3f(0.f, -1.f, 0.f), v3f(1.f, 0.f, 0.f),
                               v3f(0.f, 1.f, 0.f), v3f(0.f, 0.f, -1.f));
            } break;
            case 'd': {
              camera.Translate(v3f(1.f, 0.f, 0.f), v3f(1.f, 0.f, 0.f),
                               v3f(0.f, 1.f, 0.f), v3f(0.f, 0.f, -1.f));
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
          mouse_start = event.position;
          imui::MousePosition(event.position, imui::kEveryoneTag);
          if (!imui::MouseInUI(imui::kEveryoneTag)) mouse_down = true;
          switch (event.button) {
            case BUTTON_LEFT: {
            } break;
            case BUTTON_RIGHT: {
            } break;
            default: break;
          }
        } break;
        case MOUSE_UP: {
          mouse_down = false;
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_WHEEL: {
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
          camera.Zoom(event.wheel_delta);
        } break;
        default: break;
      }
    }
   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    UpdateGame(cursor, mouse_down, mouse_start, camera);
    RenderGame(cursor, camera, hex_map);

    const u64 elapsed_usec = platform::ClockEnd(&game_clock);
    StatsAdd(elapsed_usec, &kGameStats);

    if (kGameState.frame_target_usec > elapsed_usec) {
      u64 wait_usec = kGameState.frame_target_usec - elapsed_usec;
      platform::Clock wait_clock;
      platform::ClockStart(&wait_clock);
      while (platform::ClockEnd(&wait_clock) < wait_usec) {}
    }

    kGameState.game_time_usec += platform::ClockEnd(&game_clock);
    kGameState.game_updates++;
  }

  return 0;
}
