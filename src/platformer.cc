// Singleplayer game template.
#define SINGLE_PLAYER

// Engine stuff.
#include "math/math.cc"
#include "audio/audio.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "renderer/imui.cc"
#define PHYSICS_PARTICLE_COUNT 128
#include "physics/physics.cc"
#include "util/cooldown.cc"

// Gameplay stuff.
#include "mood/constants.cc"
#include "mood/sim.cc"
#include "mood/interaction.cc"

#define WIN_ATTACH_DEBUGGER 0

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

static r32 kJumpForce = 10000.f;

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

  mood::EntityViewer(screen);

  physics::DebugUI(screen);
}

void
GameInitialize(const v2f& dims)
{
  rgg::GetObserver()->projection = rgg::DefaultPerspective(dims);

  rgg::Camera camera;
  camera.position = v3f(0.f, 1.f, 100.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraBrowser;
  camera.speed = v3f(5.f, 5.f, 5.f);
  camera.viewport = dims;
  rgg::CameraInit(camera);

  mood::SimInitialize();
}

void
GameUpdate()
{
  // Execute game code.
  DebugUI();
  rgg::CameraUpdate();
  rgg::GetObserver()->view = rgg::CameraView();

  mood::SimUpdate();
}

void
GameRender()
{
  rgg::DebugRenderPrimitives();
  physics::DebugRender(); 
  for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
    physics::Particle2d* p = &physics::kParticle2d[i];
    if (p == mood::Player()->particle()) {
      rgg::RenderLineRectangle(p->aabb(), rgg::kGreen);
    } else {
      rgg::RenderLineRectangle(p->aabb(), rgg::kRed);
    }
  }
  imui::Render(imui::kEveryoneTag);
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  printf("Client target usec %lu\n", kGameState.frame_target_usec);

  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  while (1) {
    platform::ClockStart(&kGameState.game_clock);

    imui::ResetTag(imui::kEveryoneTag);
    rgg::DebugReset();

    if (window::ShouldClose()) break;

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      rgg::CameraUpdateEvent(event);
      physics::Particle2d* particle = mood::Player()->particle();
      switch(event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 27 /* ESC */: {
              exit(1);
            } break;
            case 'h': {
              particle->acceleration.x = -150.f;
            } break;
            case 'j': {
            } break;
            case 'k': {
              if (particle->on_ground) {
                particle->force.y = kJumpForce;
              }
            } break;
            case 'l': {
              particle->acceleration.x = 150.f;
            } break;
          }
        } break;
        case KEY_UP: {
          switch (event.key) {
            case 'h': {
              particle->acceleration.x = 0.f;
            } break;
            case 'j': {
              particle->acceleration.y = 0.f;
            } break;
            case 'k': {
              particle->acceleration.y = 0.f;
            } break;
            case 'l': {
              particle->acceleration.x = 0.f;
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
          if (!imui::MouseInUI(cursor, imui::kEveryoneTag)) {
            physics::Particle2d* p = physics::CreateParticle2d(
                rgg::CameraRayFromMouseToWorld(cursor, 0.f).xy(),
                v2f(5.f, 5.f));
            p->inverse_mass = 0.f;
          }
        } break;
        case MOUSE_UP: {
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_WHEEL: {
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
        } break;
        case XBOX_CONTROLLER: {
          if (FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_A) &&
              particle->on_ground) {
            particle->force.y += kJumpForce;
          }
          // TODO: Calculate controller deadzone with min magnitude.
          constexpr r32 kInputDeadzone = 4000.f;
          constexpr r32 kMaxControllerMagnitude = 32767.f;
          v2f stick(event.controller.lstick_x, event.controller.lstick_y);
          r32 magnitude = math::Length(stick);
          v2f nstick = stick / magnitude;
          r32 normalized_magnitude = 0;
          if (magnitude > kInputDeadzone) {
            if (magnitude > kMaxControllerMagnitude) {
              magnitude = kMaxControllerMagnitude;
            }
            magnitude -= kInputDeadzone;
            normalized_magnitude =
                magnitude / (kMaxControllerMagnitude - kInputDeadzone);
            if (nstick.x > 0.f) {
              particle->acceleration.x = 150.f * normalized_magnitude;
            } else if (nstick.x < 0.f) {
              particle->acceleration.x = -150.f * normalized_magnitude;
            }
          } else {
            magnitude = 0.0;
            normalized_magnitude = 0.0;
            particle->acceleration.x = 0.f;
          }

          if (FLAGGED(event.controller.controller_flags, XBOX_CONTROLLER_X) &&
              util::CooldownReady(&mood::kSim.boost_cooldown) &&
              magnitude > 0.0f) {
            util::CooldownReset(&mood::kSim.boost_cooldown);
            particle->force += nstick * 10000.f;
          }

          if (event.controller.right_trigger &&
              util::CooldownReady(&mood::kSim.weapon_cooldown)) {
            util::CooldownReset(&mood::kSim.weapon_cooldown);
            mood::ProjectileCreate(particle->position, v2f(1.0, 0),
                                   mood::kProjectileLaser);
          }
        } break;
        default: break;
      }
    }

    GameUpdate();
    GameRender();  
    
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
