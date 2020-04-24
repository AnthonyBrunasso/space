// Singleplayer game template.
#define SINGLE_PLAYER

#include "math/math.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"
#include "audio/audio.cc"

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

static State kGameState;
static Stats kGameStats;

#define UIBUFFER_SIZE 64
static char kUIBuffer[UIBUFFER_SIZE];

void
DebugUI()
{
  v2f screen = window::GetWindowSize();
  {
    static bool enable_debug = false;
    static v2f diagnostics_pos(3.f, screen.y);
    static float right_align = 130.f;
    imui::PaneOptions options;
    options.max_width = 315.f;
    imui::Begin("Diagnostics", imui::kEveryoneTag, options, &diagnostics_pos,
                &enable_debug);
    imui::TextOptions debug_options;
    debug_options.color = gfx::kWhite;
    debug_options.highlight_color = gfx::kRed;
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
    imui::End();
  }

  {
    static bool enable_debug = false;
    static v2f ui_pos(300.f, screen.y);
    imui::DebugPane("UI Debug", imui::kEveryoneTag, &ui_pos, &enable_debug);
  }
}

int
main(int argc, char** argv)
{
  if (!gfx::Initialize(kGameState.window_create_info)) {
    return 1;
  }

  if (!audio::Initialize()) {
    printf("Unable to initialize audio system.\n");
    return 1;
  }


  rgg::GetObserver()->projection =
      rgg::DefaultPerspective(window::GetWindowSize());

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

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch(event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 27 /* ESC */: {
              exit(1);
            } break;
          }
        case MOUSE_DOWN:
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
          break;
        case MOUSE_UP:
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
          break;
        case MOUSE_WHEEL:
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
          break;
        }
      }
    }
    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    // Execute game code.
    DebugUI();
    
    // Render
    rgg::RenderLineCube(Cubef(v3f(10.f, 30.f, -100.f), v3f(10.f, 10.f, 10.f)),
                        v4f(1.f, 0.f, 0.f, 1.f));

    rgg::RenderSphere(v3f(-10.f, -25.f, -100.f), v3f(5.f, 5.f, 5.f),
                      v4f(0.f, 1.f, 0.f, 1.f));

    rgg::DebugRenderPrimitives();

    imui::Render(imui::kEveryoneTag);
    
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
