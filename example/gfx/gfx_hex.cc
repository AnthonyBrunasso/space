// Singleplayer game template.
#define SINGLE_PLAYER

#include <vector>

#include "math/math.cc"

#include "audio/audio.cc"
#include "renderer/camera.cc"
#include "renderer/renderer.cc"
#include "renderer/imui.cc"

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

#define UIBUFFER_SIZE 64
static char kUIBuffer[UIBUFFER_SIZE];

void
DebugUI()
{
  v2f screen = window::GetWindowSize();
  {
    static b8 enable_debug = false;
    static v2f diagnostics_pos(3.f, screen.y);
    static r32 right_align = 130.f;
    imui::PaneOptions options;
    options.max_width = 315.f;
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
    imui::End();
  }

  {
    static b8 enable_debug = false;
    static v2f ui_pos(300.f, screen.y);
    imui::DebugPane("UI Debug", imui::kEveryoneTag, &ui_pos, &enable_debug);
  }
}

void
RenderHexGridCoord(v2i cord, bool color_swap = false)
{
  v2f world = math::HexAxialToWorld(cord, 5.f);
  rgg::RenderHexagon(v3f(world.x, world.y, -50.f), v3f(1.f, 1.f, 1.f),
                       v4f(.2f, .2f, .2f, 1.f));
  rgg::RenderLineHexagon(v3f(world.x, world.y, -50.f), 5.f, rgg::kWhite);
  //rgg::RenderCube(Cubef(world.x, world.y, -50.f, 1.f, 1.f, 1.f), rgg::kWhite);
  if (color_swap) {
    rgg::RenderLineHexagon(v3f(world.x, world.y, -49.5f), 5.f, rgg::kRed);
    rgg::RenderLineHexagon(v3f(world.x, world.y, -49.0f), 5.f, rgg::kRed);
    rgg::RenderLineHexagon(v3f(world.x, world.y, -48.5f), 5.f, rgg::kRed);
  }
}

s32
main(s32 argc, char** argv)
{
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

  if (!audio::Initialize()) {
    printf("Unable to initialize audio system.\n");
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

  std::vector<v2i> grids = math::HexAxialRange(10);

  bool mouse_down = false;
  v2f mouse_start;

  v3f wri(1.f, 0.f, 0.f);
  v3f wup(0.f, 1.f, 0.f);
  v3f wfo(0.f, 0.f, -1.f);

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
              camera.Translate(v3f(0.f, 1.f, 0.f), wri, wup, wfo);
            } break;
            case 'a': {
              camera.Translate(v3f(-1.f, 0.f, 0.f), wri, wup, wfo);
            } break;
            case 's': {
              camera.Translate(v3f(0.f, -1.f, 0.f), wri, wup, wfo);
            } break;
            case 'd': {
              camera.Translate(v3f(1.f, 0.f, 0.f), wri, wup, wfo);
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          mouse_down = true;
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
          mouse_start = event.position;
        } break;
        case MOUSE_UP: {
          mouse_down = false;
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_WHEEL: {
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
          camera.Zoom(event.wheel_delta);
        } break;
      }
    }
   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    if (mouse_down) {
      v2f diff = cursor - mouse_start;
      if (fabs(diff.x) > 0.f || fabs(diff.y) > 0.f) {
        camera.PitchYawDelta(diff.yx() * 0.1f);
        mouse_start = cursor;
      }
    }

    rgg::GetObserver()->view = camera.View();

    v3f p = camera.RayFromScreenToWorld(cursor, window::GetWindowSize(), 50.f);

    v2i picked = HexWorldToAxial(p.xy(), 5.f);

    for (auto g : grids) {
      RenderHexGridCoord(g, g == picked);
    }

    
    //rgg::RenderCube(Cubef(p, 1.f, 1.f, 1.f), rgg::kGreen);

    //camera.DebugRender();

    // Execute game code.
    DebugUI();

    rgg::DebugRenderPrimitives();

    imui::Render(imui::kEveryoneTag);
    
    window::SwapBuffers();

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
