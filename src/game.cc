// Singleplayer game template.
#define SINGLE_PLAYER

#include "math/math.cc"

#include "audio/audio.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
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

static Rectf kRect1;
static Rectf kRect2;

void
DebugUI()
{
  v2f screen = window::GetWindowSize();
  {
    static b8 enable_debug = true;
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
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Rect Distance");
    snprintf(kUIBuffer, sizeof(kUIBuffer), "%f",
             math::DistanceBetween(kRect2, kRect1));
    imui::Text(kUIBuffer);
    imui::NewLine();

    imui::End();
  }
#if 0
  {
    static b8 enable_debug = false;
    static v2f ui_pos(300.f, screen.y);
    imui::DebugPane("UI Debug", imui::kEveryoneTag, &ui_pos, &enable_debug);
  }
#endif
}

s32
main(s32 argc, char** argv)
{
  platform::Clock game_clock;

  if (!memory::Initialize(MiB(64))) {
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

  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  rgg::Camera camera(v3f(0.f, 0.f, 100.f), v3f(0.f, 1.f, 0.f));

  rgg::GetObserver()->projection =
      rgg::DefaultPerspective(window::GetWindowSize());


  kRect1 = Rectf(0.f, 0.f, 10.f, 10.f);
  kRect2 = Rectf(-40.f, 10.f, 10.f, 10.f);

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
          }
        case MOUSE_DOWN: {
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
          v3f world = camera.RayFromScreenToWorld(
              event.position, window::GetWindowSize(), 0.f);
          kRect2.x = world.x;
          kRect2.y = world.y;
        } break;
        case MOUSE_UP:
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
          break;
        case MOUSE_WHEEL:
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
          break;
        }
      }
    }
   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    rgg::GetObserver()->view = camera.View();

        
    // Render
    //rgg::RenderLineCube(Cubef(v3f(10.f, 30.f, -100.f), v3f(10.f, 10.f, 10.f)),
    //                    v4f(1.f, 0.f, 0.f, 1.f));

    rgg::RenderRectangle(kRect1, 0.f, 0.f, v4f(0.f, .2f, .8f, 1.f));
    rgg::RenderRectangle(kRect2, 0.f, 0.f, v4f(0.f, .8f, .2f, 1.f));

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
