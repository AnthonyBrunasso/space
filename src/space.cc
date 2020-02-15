#include <cassert>
#include <cstdio>

#include "math/math.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"
#include "simulation/camera.cc"
#include "simulation/simulation.cc"

struct State {
  // Game and render updates per second
  uint64_t framerate = 60;
  // Calculated available microseconds per game_update
  uint64_t frame_target_usec;
  // Game clock state
  Clock_t game_clock;
  // Time it took to run a frame.
  uint64_t frame_time_usec = 0;
  Stats stats;
  // Input determinism check
  uint64_t input_hash = DJB2_CONST;
  uint64_t simulation_hash = DJB2_CONST;
  // Rough estimate of round-trip time
  uint64_t rtt_usec = 0;
  uint64_t turn_queue_depth = 0;
  // Allow yielding idle cycles to kernel
  bool sleep_on_loop = true;
  // Number of times the game has been updated.
  uint64_t game_updates = 0;
  uint64_t logic_updates = 0;
  // Number of times the game frame was exceptionally delayed
  uint64_t game_jerk = 0;
  // TODO (AN): Find a home in simulation/
  Camera player_camera[MAX_PLAYER];
};

static State kGameState;

// TODO (AN): Revisit cameras
const Camera*
GetLocalCamera()
{
  return &kGameState.player_camera[kNetworkState.player_id];
}

void
ProcessWindowInput(InputBuffer* input_buffer)
{
  uint64_t event_count = 0;
  while (event_count < MAX_TICK_EVENTS) {
    PlatformEvent pevent;
    if (!window::PollEvent(&pevent)) break;

    uint64_t type = pevent.type;
    switch (type) {
      case MOUSE_DOWN:
      case MOUSE_UP:
      case KEY_DOWN:
      case KEY_UP:
        input_buffer->input_event[event_count] = pevent;
        ++event_count;
        break;
    }
  }

  input_buffer->used_input_event = event_count;
}

void
ProcessInput()
{
  InputBuffer* buffer = GetNextInputBuffer();
#ifndef HEADLESS
  ProcessWindowInput(buffer);
#else
  buffer->used_input_event = 0;
#endif
}

void
SetProjection()
{
  // TODO (AN): revisit camera assumptions
#ifdef HEADLESS
  v2f size = {1280.0f, 720.0f};
#else
  v2f size = window::GetWindowSize();
#endif
  rgg::GetObserver()->projection = math::Ortho(
      size.x, 0.f, size.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
}

v3f
CoordToWorld(v2f xy)
{
  auto dims = window::GetWindowSize();
  return camera::ScreenToWorldSpace(GetLocalCamera(), v3f(xy - dims * 0.5f));
}

void
SimulationEvent(const PlatformEvent* event, const Camera* camera,
                v2f* translation)
{
  djb2_hash_more((const uint8_t*)event, sizeof(PlatformEvent),
                 &kGameState.input_hash);

  switch (event->type) {
    case MOUSE_DOWN: {
      imui::MouseClick(event->position);
      v3f pos = CoordToWorld(event->position);

      if (event->button == BUTTON_LEFT) {
        // Control
        uint64_t unit = simulation::SelectUnit(pos);
        simulation::ControlUnit(unit);
      } else if (event->button == BUTTON_RIGHT) {
        // Move
        Command command = {Command::kMove, pos.xy()};
        PushCommand(command);
      }
    } break;
    case KEY_DOWN: {
      switch (event->key) {
        case 'w': {
          translation->y = 1.f;
        } break;
        case 'a': {
          translation->x = -1.f;
        } break;
        case 's': {
          translation->y = -1.f;
        } break;
        case 'd': {
          translation->x = 1.f;
        } break;
        case 'm': {
          v3f pos = CoordToWorld(event->position);
          simulation::PlaceModuleMine(pos);
        } break;
        default:
          break;
      }
    } break;
    case KEY_UP: {
      switch (event->key) {
        case 'w': {
          translation->y = 0.f;
        } break;
        case 'a': {
          translation->x = 0.f;
        } break;
        case 's': {
          translation->y = 0.f;
        } break;
        case 'd': {
          translation->x = 0.f;
        } break;
        default:
          break;
      }
    } break;
    default:
      break;
  }
}

void
ProcessSimulation(int player_id, uint64_t event_count,
                  const PlatformEvent* event)
{
  // Shared player control of the ship for now
  for (int i = 0; i < event_count; ++i) {
    SimulationEvent(&event[i], &kGameState.player_camera[player_id],
                    &kGameState.player_camera[player_id].motion);
  }
}

int
main(int argc, char** argv)
{
  while (1) {
    int opt = platform_getopt(argc, argv, "i:p:n:");
    if (opt == -1) break;

    switch (opt) {
      case 'i':
        kNetworkState.server_ip = platform_optarg;
        break;
      case 'p':
        kNetworkState.server_port = platform_optarg;
        break;
      case 'n':
        kNetworkState.num_players = strtol(platform_optarg, NULL, 10);
        break;
    }
  }
  printf("Client will connect to game at %s:%s\n", kNetworkState.server_ip,
         kNetworkState.server_port);

#ifndef HEADLESS
  // Platform & Gfx init
  if (!gfx::Initialize()) {
    return 1;
  }
#endif

  // Camera init
  for (int i = 0; i < MAX_PLAYER; ++i) {
    camera::InitialCamera(&kGameState.player_camera[i]);
  }
  camera::SetView(GetLocalCamera(), &rgg::GetObserver()->view);

  // Projection init
  SetProjection();

  // Game init
  if (!simulation::Initialize()) {
    return 1;
  }

  // Network handshake uses a clock
  if (!NetworkSetup()) {
    return 1;
  }

  // main thread affinity set to core 0
  if (platform::thread_affinity_count() > 1) {
    platform::thread_affinity_set(0);
    printf("Game thread may run on %d cores\n",
           platform::thread_affinity_count());
  }

  uint64_t bytes = 0;
  uint64_t min_ptr = UINT64_MAX;
  uint64_t max_ptr = 0;
  for (int i = 0; i < kUsedRegistry; ++i) {
    printf("Registry ptr %p\n", kRegistry[i].ptr);
    bytes += kRegistry[i].memb_max * kRegistry[i].memb_size;
    max_ptr = MAX((uint64_t)kRegistry[i].ptr, max_ptr);
    min_ptr = MIN((uint64_t)kRegistry[i].ptr, min_ptr);
  }
  printf("[min page 0x%lx] [max page 0x%lx] [page_count %lu]\n",
         ANDN(PAGE - 1, min_ptr), ANDN(PAGE - 1, max_ptr),
         1 + ((ANDN(PAGE - 1, max_ptr) - ANDN(PAGE - 1, min_ptr)) / PAGE));

  // Reset State
  StatsInit(&kGameState.stats);
  kGameState.game_updates = 0;
  kGameState.game_jerk = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;

#ifndef HEADLESS
  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
  // Reset the clock for simulation
  platform::clock_init(kGameState.frame_target_usec, &kGameState.game_clock);
  while (!window::ShouldClose()) {
    ProcessInput();
    kGameState.rtt_usec = NetworkEgress() * kGameState.frame_target_usec;
    NetworkIngress(kGameState.logic_updates);
    kGameState.turn_queue_depth = NetworkReadyCount();

    uint64_t slot = NETQUEUE_SLOT(kGameState.logic_updates);
    if (SlotReady(slot)) {
      // Hash the simulation state every 0th slot
      if (!slot)
        if (!simulation::VerifyIntegrity(&kGameState.simulation_hash)) exit(4);

      // Game Mutation: Apply player commands for turn N
      InputBuffer* game_turn = GetSlot(slot);
      for (int i = 0; i < MAX_PLAYER; ++i) {
        InputBuffer* player_turn = &game_turn[i];
        ProcessSimulation(i, player_turn->used_input_event,
                          player_turn->input_event);
      }

      // Game Mutation: continue simulation
      simulation::Update();

      // Camera
      for (int i = 0; i < MAX_PLAYER; ++i) {
        camera::Update(&kGameState.player_camera[i]);
      }
      camera::SetView(GetLocalCamera(), &rgg::GetObserver()->view);

      // Give the user an update tick. The engine runs with
      // a fixed delta so no need to provide a delta time.
      ++kGameState.logic_updates;
    }

    // Misc debug/feedback
    auto sz = window::GetWindowSize();
#define BUFFER_SIZE 64
    char buffer[BUFFER_SIZE];
    static bool enable_debug = false;
    auto mouse = CoordToWorld(window::GetCursorPosition());
    imui::PaneOptions options;
    imui::Begin(v2f(3.f, sz.y - 30.f), options);
    imui::TextOptions debug_options;
    debug_options.color = gfx::kWhite;
    debug_options.highlight_color = gfx::kRed;
    if (imui::Text("Debug (Click to expand)", debug_options).clicked) {
      enable_debug = !enable_debug;
    }

    if (enable_debug) {
      snprintf(buffer, BUFFER_SIZE, "Frame Time: %04.02f us [%02.02f%%]",
               StatsMean(&kGameState.stats),
               100.f * StatsUnbiasedRsDev(&kGameState.stats));
      imui::Indent(2);
      imui::Text(buffer);
      snprintf(buffer, BUFFER_SIZE, "Network Rtt: %06lu us [%lu/%lu queue]",
               kGameState.rtt_usec, kGameState.turn_queue_depth, MAX_NETQUEUE);
      imui::Text(buffer);
      snprintf(buffer, BUFFER_SIZE, "Window Size: %ix%i", (int)sz.x, (int)sz.y);
      imui::Text(buffer);
      snprintf(buffer, BUFFER_SIZE, "Mouse Pos In World: (%.1f,%.1f)", mouse.x,
               mouse.y);
      imui::Text(buffer);
      snprintf(buffer, BUFFER_SIZE, "Input hash: 0x%lx", kGameState.input_hash);
      imui::Text(buffer);
      snprintf(buffer, BUFFER_SIZE, "Sim hash: 0x%lx",
               kGameState.simulation_hash);
      imui::Text(buffer);
      v2i tile = simulation::WorldToTilePos(mouse.xy());
      if (simulation::TileOk(tile)) {
        snprintf(buffer, BUFFER_SIZE, "Type: %d",
                 simulation::kTilemap.map[tile.y][tile.x].type);
        imui::Text(buffer);
      }
      imui::Indent(-2);
    }
    snprintf(buffer, BUFFER_SIZE, "Minerals: %lu", kShip[0].mineral);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Level: %lu", kShip[0].level);
    imui::Text(buffer);

    if (simulation::SimulationOver()) {
      snprintf(buffer, BUFFER_SIZE, "Game Over");
      imui::Text(buffer);
    } else if (simulation::ShipFtlReady()) {
      if (imui::Button(math::Rect(10, 10, 40, 40), v4f(1.0f, 0.0f, 1.0f, 0.75f))
              .clicked) {
        simulation::ShipFtlInit();
      }
    }

    imui::End();

    v2f pos;
    if (imui::GetUIClick(&pos)) {
      LOGFMT("ui click event pos (%.2f, %.2f)", pos.x, pos.y);
    }

    imui::PaneOptions pane_options(300.0f, 100.0f);
    imui::TextOptions text_options;
    text_options.scale = 0.7f;
    imui::Begin(v2f(0.f, 0.f), pane_options);
    for (int i = 0, imax = LogCount(); i < imax; ++i) {
      const char* log_msg = ReadLog(i);
      if (!log_msg) continue;
      imui::Text(log_msg, text_options);
    }
    imui::End();

#ifndef HEADLESS
    // The bottom left and top right of the screen with regards to the camera.
    const Camera* cam = GetLocalCamera();
    const v2f dims = window::GetWindowSize();
    v3f top_right = CoordToWorld(dims);
    v3f bottom_left = CoordToWorld({0.f, 0.f});
    gfx::Render(math::Rectf{bottom_left.xy(), top_right.xy()}, mouse.xy(),
                dims);
#endif
    imui::Reset();

    // Capture frame time before the potential stall on vertical sync
    const uint64_t elapsed_usec = platform::delta_usec(&kGameState.game_clock);
    kGameState.frame_time_usec = elapsed_usec;
    StatsAdd(elapsed_usec, &kGameState.stats);

#ifndef HEADLESS
    window::SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

#if 0
    printf("[frame %lu]\n", kGameState.game_updates);
#endif
    ++kGameState.game_updates;

    uint64_t sleep_usec = 0;
    uint64_t sleep_count = kGameState.sleep_on_loop;
    while (!platform::clock_sync(&kGameState.game_clock, &sleep_usec)) {
      while (sleep_count) {
        --sleep_count;
        platform::sleep_usec(sleep_usec);
      };
    }
  }

  return 0;
}
