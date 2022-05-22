#include "common/include.cc"

struct GameState {
  // Lock the framerate to this value.
  u32 framerate = 60;
  // Available microseconds per game update
  u64 framerate_usec;
  // Estimate of gime passed since game start.
  u64 game_time_usec = 0;
  // Cuz my windows machine gots a bigger montior
#ifdef _WIN32
  u32 window_width = 1920;
  u32 window_height = 1080;
#else
  u32 window_width = 1600;
  u32 window_height = 900;
#endif
  // Setting this to true does nice things for battery life / fan noise on laptops
  // but is a significant perf hit.
  b8 sleep_on_wait = true;
};

static GameState kGameState;
static Stats kGameStats;

#include "editor/editor.cc"

static bool kShowDemoWindow = true;

bool SetupWorkingDirectory() {
  bool result = false;
  // Check the directory the binary is run from and one backwards then give up.
  if (!filesystem::WorkingDirectoryContains("asset")) {
#ifdef _WIN32
    filesystem::ChangeDirectory("..\\");
#else
    filesystem::ChangeDirectory("../");
#endif
    result = filesystem::WorkingDirectoryContains("asset"); 
  } else {
    result = true;
  }
  return result;
}

s32 main(s32 argc, char** argv) {
  kEditor.window_create_info.window_width = kGameState.window_width;
  kEditor.window_create_info.window_height = kGameState.window_height;

  kGameState.framerate_usec = 1000 * 1000 / kGameState.framerate;

  if (!SetupWorkingDirectory()) {
    LOG(ERR, "Unable to setup working directory.");
    return 1;
  }

  LOG(INFO, "Working dir: %s", filesystem::GetWorkingDirectory());

  if (!window::Create("Space", kEditor.window_create_info)) {
    return 1;
  }

  if (!rgg::Initialize()) {
    return 1;
  }

  LOG(INFO, "Client target usec %lu", kGameState.framerate_usec);

  platform::Clock game_clock;

  while (1) {
    platform::ClockStart(&game_clock);

    if (window::ShouldClose()) break;
    ImGuiImplNewFrame();
    ImGui::NewFrame();

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      ImGuiImplProcessEvent(event);
      EditorProcessEvent(event);
    }

    // TODO: Abstract into renderer calls.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.1f, .1f, .13f, 1.f);

    EditorMain();
    
    if (kShowDemoWindow) {
      ImGui::ShowDemoWindow(&kShowDemoWindow);
    }

    rgg::DebugRenderUIPrimitives();

    ImGui::Render();
    ImGuiImplRenderDrawData();
    ImGui::EndFrame();

    window::SwapBuffers();

    const u64 elapsed_usec = platform::ClockEnd(&game_clock);
    StatsAdd(elapsed_usec, &kGameStats);

    if (kGameState.framerate_usec > elapsed_usec) {
      u64 wait_usec = kGameState.framerate_usec - elapsed_usec;
      if (kGameState.sleep_on_wait) {
        platform::SleepUsec(wait_usec);
      } else {
        platform::Clock wait_clock;
        platform::ClockStart(&wait_clock);
        while (platform::ClockEnd(&wait_clock) < wait_usec) {}
      }
    }

    kGameState.game_time_usec += platform::ClockEnd(&game_clock);
  }

  return 0;
}
