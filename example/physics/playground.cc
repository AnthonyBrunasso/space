// Singleplayer game template.
#define SINGLE_PLAYER

#include "math/math.cc"

#include "audio/audio.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "renderer/imui.cc"

#define PHYSICS_PARTICLE_COUNT 64
#include "physics/physics.cc"

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

static physics::Particle2d* kParticle = nullptr;

static char kUIBuffer[64];

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

  {
    static b8 enable_physics = true;
    static v2f physics_pos(0.f, screen.y - 300.f);
    imui::PaneOptions options;
    options.width = options.max_width = 315.f;
    imui::Begin("Physics", imui::kEveryoneTag, options, &physics_pos,
                &enable_physics);
    imui::Space(imui::kVertical, 3.f);
    for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
      physics::Particle2d* p = &physics::kParticle2d[i];
      imui::Text("Particle");
      imui::Indent(2);
      static const r32 kWidth = 120.f;
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Position");
      snprintf(kUIBuffer, sizeof(kUIBuffer), "%.3f,%.3f", p->position.x,
               p->position.y);
      imui::Text(kUIBuffer);
      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Velocity");
      snprintf(kUIBuffer, sizeof(kUIBuffer), "%.3f,%.3f", p->velocity.x,
               p->velocity.y);
      imui::Text(kUIBuffer);
      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Acceleration");
      snprintf(kUIBuffer, sizeof(kUIBuffer), "%.3f,%.3f", p->acceleration.x,
               p->acceleration.y);
      imui::Text(kUIBuffer);
      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Inverse Mass");
      snprintf(kUIBuffer, sizeof(kUIBuffer), "%.3f", p->inverse_mass);
      imui::Width(kWidth / 2.f);
      imui::Text(kUIBuffer);
      if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
        p->inverse_mass -= .1f;
      }
      imui::Space(imui::kHorizontal, 5.f);
      if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
        p->inverse_mass += .1f;
      }
      imui::NewLine();
      imui::SameLine();
      imui::Width(kWidth);
      imui::Text("Damping");
      snprintf(kUIBuffer, sizeof(kUIBuffer), "%.3f", p->damping);
      imui::Width(kWidth / 2.f);
      imui::Text(kUIBuffer);
      if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
        p->damping -= .05f;
      }
      imui::Space(imui::kHorizontal, 5.f);
      if (imui::Button(16.f, 16.f, rgg::kBlue).clicked) {
        p->damping += .05f;
      }
      p->damping = CLAMPF(p->damping, 0.f, 1.0f);
      imui::NewLine();
      imui::Indent(-2);
    }
    imui::End();
  }

}

void
GameInitialize()
{
  rgg::GetObserver()->projection = rgg::DefaultOrtho(window::GetWindowSize());

  rgg::Camera camera;
  camera.position = v3f(0.f, 1.f, -.85f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraBrowser;
  camera.speed = v3f(5.f, 5.f, 0.01f);
  rgg::CameraInit(camera);

  kParticle = physics::UseParticle2d();
  kParticle->dims = v2f(10.f, 10.f);
}

void
GameUpdate()
{
  // Execute game code.
  DebugUI();
  rgg::CameraUpdate();
  rgg::GetObserver()->view = rgg::CameraView();
  physics::Integrate(1.f);
}

void
GameRender()
{
  for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
    physics::Particle2d* p = &physics::kParticle2d[i];
    rgg::RenderLineRectangle(p->aabb(), rgg::kRed);
    rgg::RenderCircle(p->position, 0.5f, rgg::kGreen);
  }
  rgg::DebugRenderPrimitives();
  imui::Render(imui::kEveryoneTag);
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

  GameInitialize();
  
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

  while (1) {
    platform::ClockStart(&game_clock);

    imui::ResetTag(imui::kEveryoneTag);
    rgg::DebugReset();

    if (window::ShouldClose()) break;

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      rgg::CameraUpdateEvent(event);
      switch(event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 27 /* ESC */: {
              exit(1);
            } break;
            case 'j': {
              kParticle->acceleration.x = -0.5f;
            } break;
            case 'l': {
              kParticle->acceleration.x = 0.5f;
            } break;
          }
        } break;
        case KEY_UP: {
          switch (event.key) {
            case 'j': {
              kParticle->acceleration.x = 0.f;
            } break;
            case 'l': {
              kParticle->acceleration.x = 0.f;
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_UP: {
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_WHEEL: {
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
        } break;
        default: break;
      }
    }

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);
    
    GameUpdate();
    GameRender();  
    
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
