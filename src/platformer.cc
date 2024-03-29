// Singleplayer game template.
#define SINGLE_PLAYER

#include "common/include.cc"

#define PHYSICS_PARTICLE_COUNT 2048
#include "physics/physics.cc"

// Gameplay stuff.
#include "mood/constants.cc"
#include "mood/components.cc"
#include "mood/anim.cc"
#include "mood/player.cc"
#include "mood/util.cc"
#include "mood/render.cc"
#include "mood/character.cc"
#include "mood/sim.cc"
#include "mood/spawner.cc"
#include "mood/obstacle.cc"
#include "mood/map.cc"
#include "mood/interaction.cc"

#define WIN_ATTACH_DEBUGGER 0
#define DEBUG_PHYSICS 0
#define DEBUG_UI 1
#define PLATFORMER_CAMERA 1

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

static char kUIBuffer[64];

void
SetFramerate(u64 fr)
{
  kGameState.framerate = fr;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
}

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
    imui::SameLine();
    imui::Width(right_align);
    imui::Text("Game Speed");
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
    imui::End();
  }

  {
#if DEBUG_UI
    static b8 enable_debug = false;
    static v2f ui_pos(300.f, screen.y);
    imui::DebugPane("UI Debug", imui::kEveryoneTag, &ui_pos, &enable_debug);
#endif
  }

  //physics::DebugUI(screen);
  mood::EntityViewer(screen);
  mood::MapEditor(screen);
#if DEBUG_PHYSICS
  static b8 enable = false;
  physics::DebugUI(screen, &enable);
#endif
}

void
GameInitialize(const v2f& dims)
{
  rgg::GetObserver()->projection =
    //math::Perspective(64.f, dims.x / dims.y, 1.f, 1000.f);
    math::Ortho(dims.x, 0.f, dims.y, 0.f, -1.f, 1.f);

  if (!rgg::CameraHasLocalCamera()) {
    rgg::Camera camera;
    camera.position = v3f(0.f, 0.f, 0.f);
    camera.dir = v3f(0.f, 0.f, -1.f);
    camera.up = v3f(0.f, 1.f, 0.f);
    camera.mode = rgg::kCameraBrowser;
    camera.speed = v3f(5.f, 5.f, 0.0f);
    camera.viewport = dims;
    camera.camera_control = false;
    rgg::CameraInit(camera);
  }

  mood::SimInitialize();
  mood::RenderInitialize();
  mood::InteractionInitialize();

  if (strlen(mood::kCurrentMapName) > 0) {
    mood::MapLoadFrom(mood::kCurrentMapName);
  }
}

bool
GameUpdate()
{
  mood::RenderUpdate();
  // Execute game code.
  DebugUI();
  rgg::CameraUpdate();
  rgg::GetObserver()->view = rgg::CameraView();
  if (mood::kReloadGame) return true;
  if (mood::kFreezeGame) return false;
  if (mood::kEditMode) return false;
  else {
    // Allows FrameCooldown to work based on # of sim updates.
    util::FrameCooldownUpdate();
    return mood::SimUpdate();
  }
}

void
GameRender(v2f dims)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(.1f, .1f, .13f, 1.f);
  static rgg::Texture render_target =
      rgg::CreateEmptyTexture2D(GL_RGB, mood::kRenderTargetWidth,
                                mood::kRenderTargetHeight);
  {
    PIXEL_ART_OBSERVER();
    rgg::BeginRenderTo(render_target);
    glClear(GL_COLOR_BUFFER_BIT);
    mood::Render();
#if DEBUG_PHYSICS
    physics::DebugRender();
#endif
    rgg::EndRenderTo();
  }

  rgg::GetObserver()->view =
    math::LookAt(v3f(0.f, 0.f, .5f), v3f(0.f, 0.f, 0.f), v3f(0.f, 1.f, 0.f));

  static bool render_game = true;
  ImGui::Begin("Game", &render_game);
  ImGui::Image((void*)(intptr_t)render_target.reference,
               ImVec2(mood::kRenderTargetWidth, mood::kRenderTargetHeight));
  ImGui::End();

  /*rgg::RenderTexture(
      render_target,
      Rectf(0.f, 0.f, mood::kRenderTargetWidth, mood::kRenderTargetHeight),
      Rectf(-mood::kScreenWidth / 2.f, -mood::kScreenHeight / 2.f,
            mood::kScreenWidth, mood::kScreenHeight), true);*/

  ImGui::Render();
  ImGuiImplRenderDrawData();

  rgg::DebugRenderUIPrimitives();

  imui::Render(imui::kEveryoneTag);
  window::SwapBuffers();
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

  kGameState.window_create_info.window_width = mood::kScreenWidth;
  kGameState.window_create_info.window_height = mood::kScreenHeight;
  if (!window::Create("Game", kGameState.window_create_info)) {
    return 1;
  }


  if (!rgg::Initialize()) {
    return 1;
  }

  /*if (!audio::Initialize()) {
    printf("Unable to initialize audio system.\n");
    return 1;
  }*/

  const v2f dims = window::GetWindowSize();
  GameInitialize(dims);

  LOG(INFO, "%s", window::GetBinaryPath());
  
  // Reset State
  StatsInit(&kGameStats);
  kGameState.game_updates = 0;
  SetFramerate(kGameState.framerate);
  LOG(INFO, "Client target usec %lu", kGameState.frame_target_usec);

  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();

  bool show_demo_window = true;

  while (1) {
    platform::ClockStart(&kGameState.game_clock);

    imui::ResetTag(imui::kEveryoneTag);
    rgg::DebugReset();

    if (window::ShouldClose()) break;

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    ImGuiImplNewFrame();
    ImGui::NewFrame();

    if (show_demo_window) {
      ImGui::ShowDemoWindow(&show_demo_window);
    }

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      rgg::CameraUpdateEvent(event);
      ImGuiImplProcessEvent(event);
      mood::ProcessPlatformEvent(event, cursor);
    }

    if (GameUpdate()) {
      mood::SimReset();
      GameInitialize(dims);
    } else {
      GameRender(dims);  
    }

    ImGui::EndFrame();

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
