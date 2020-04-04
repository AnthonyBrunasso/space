#include <cassert>
#include <cstdio>

#include "math/math.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"
#include "simulation/interaction.cc"
#include "simulation/simulation.cc"

struct State {
  // Game and render updates per second
  uint64_t framerate = 60;
  // Calculated available microseconds per game_update
  uint64_t frame_target_usec;
  // Game clock state
  TscClock_t game_clock;
  // Time it took to run a frame.
  uint64_t frame_time_usec = 0;
  // (optional) limit the simulation frames (UINT64_MAX will loop infinitely)
  uint64_t frame_limit = UINT64_MAX;
  // (optional) yield unused cpu time to the system
  bool sleep_on_loop = true;
  // Number of times the game has been updated.
  uint64_t game_updates = 0;
  uint64_t logic_updates = 0;
  // Most recent frame that was unable to advance due to input loss
  uint64_t choke_frame = 0;
  // Parameters window::Create will be called with.
  window::CreateInfo window_create_info;
};

static State kGameState;
static Stats kGameStats;

// TODO (AN): Revisit cameras
const Camera*
GetCamera(uint64_t player_index)
{
  return &kPlayer[player_index].camera;
}

void
GatherWindowInput(InputBuffer* input_buffer)
{
  uint64_t event_count = 0;

  int i = 0;
  for (; i < MAX_TICK_EVENTS * 2; ++i) {
    if (event_count >= (MAX_TICK_EVENTS - 1)) break;
    PlatformEvent pevent;
    if (!window::PollEvent(&pevent)) break;

    uint64_t type = pevent.type;
    switch (type) {
      case MOUSE_DOWN:
      case MOUSE_UP:
      case MOUSE_WHEEL:
      case KEY_DOWN:
      case KEY_UP:
        input_buffer->input_event[event_count] = pevent;
        ++event_count;
        break;
    }
    switch (type) {
      case MOUSE_DOWN:
        imui::MouseClick(pevent.position, pevent.button, imui::kEveryoneTag);
        break;
    }
  }

  // Always append an estimate of the the local mouse cursor
  const v2f cursor = window::GetCursorPosition();
  input_buffer->input_event[event_count].type = MOUSE_POSITION;
  input_buffer->input_event[event_count].position = cursor;
  event_count += 1;

  imui::MousePosition(cursor, imui::kEveryoneTag);

  input_buffer->used_input_event = event_count;
}

void
GatherInput()
{
  InputBuffer* buffer = GetNextInputBuffer();

#ifndef HEADLESS
  GatherWindowInput(buffer);
#else
  buffer->used_input_event = 0;
#endif
}

void
SetProjection()
{
  // TODO (AN): revisit camera assumptions
#ifdef HEADLESS
  v2f size = {1920.0f, 1080.0f};
#else
  v2f size = window::GetWindowSize();
#endif
  rgg::GetObserver()->projection =
      math::Perspective(67.f, size.x / size.y, .1f, 2000.f);
}

int
main(int argc, char** argv)
{
  while (1) {
    int opt = platform_getopt(argc, argv, "i:p:n:f:s:w:h:x:y:");
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
      case 'f':
        kGameState.frame_limit = strtol(platform_optarg, NULL, 10);
        break;
      case 's':
        simulation::kScenario.type =
            (simulation::Scenario::Type)strtol(platform_optarg, NULL, 10);
        break;
      case 'w':
        kGameState.window_create_info.window_width =
            strtol(platform_optarg, NULL, 10);
        break;
      case 'h':
        kGameState.window_create_info.window_height =
            strtol(platform_optarg, NULL, 10);
        break;
      case 'x':
        kGameState.window_create_info.window_pos_x =
            strtol(platform_optarg, NULL, 10);
        break;
      case 'y':
        kGameState.window_create_info.window_pos_y =
            strtol(platform_optarg, NULL, 10);
        break;
    }
  }
  printf("Client will connect to game at %s:%s\n", kNetworkState.server_ip,
         kNetworkState.server_port);

#ifndef HEADLESS
  // Platform & Gfx init
  if (!gfx::Initialize(kGameState.window_create_info)) {
    return 1;
  }
#endif

  // Network handshake uses a clock
  if (!NetworkSetup()) {
    return 1;
  }

  // Game init
  kPlayerCount = kNetworkState.player_count;
  kPlayerIndex = kNetworkState.player_index;
  if (!simulation::Initialize(kNetworkState.game_id)) {
    return 1;
  }
  // Init view for local player's camera
  camera::SetView(GetCamera(kNetworkState.player_index),
                  &rgg::GetObserver()->view);

  // Projection init
  SetProjection();

  // main thread affinity set to core 0
  if (platform::thread_affinity_count() > 1) {
    platform::thread_affinity_usecore(0);
    printf("Game thread may run on %d cores\n",
           platform::thread_affinity_count());
  }

  uint64_t bytes = 0;
  uint64_t min_ptr = UINT64_MAX;
  uint64_t max_ptr = 0;
  for (int i = 0; i < kUsedRegistry; ++i) {
    printf("[%d] Registry ptr %p [%lu size] [%lu count]\n", i, kRegistry[i].ptr,
           kRegistry[i].memb_size, kRegistry[i].memb_max);
    bytes += kRegistry[i].memb_max * kRegistry[i].memb_size;
    max_ptr = MAX((uint64_t)kRegistry[i].ptr, max_ptr);
    min_ptr = MIN((uint64_t)kRegistry[i].ptr, min_ptr);
  }
  printf("EntityRegistry [min page 0x%lx] [max page 0x%lx] [page_count %lu]\n",
         ANDN(PAGE - 1, min_ptr), ANDN(PAGE - 1, max_ptr),
         1 + ((ANDN(PAGE - 1, max_ptr) - ANDN(PAGE - 1, min_ptr)) / PAGE));

  // Reset State
  StatsInit(&kGameStats);
  kGameState.game_updates = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  printf("Client target usec %lu\n", kGameState.frame_target_usec);

#ifndef HEADLESS
  // If vsync is enabled, force the clock_init to align with clock_sync
  // TODO: We should also enforce framerate is equal to refresh rate
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
  // Reset the clock for simulation
  clock_init(kGameState.frame_target_usec, &kGameState.game_clock);
  printf("median_tsc_per_usec %lu\n", median_tsc_per_usec);
  const uint64_t frame_limit = kGameState.frame_limit;
  uint64_t frame = 0;
  for (; frame <= frame_limit; ++frame) {
    if (window::ShouldClose()) break;
    imui::ResetTag(imui::kEveryoneTag);

    GatherInput();
    NetworkEgress();
    NetworkIngress(kGameState.logic_updates);
    if (kNetworkExit) break;

    const int frame_queue =
        NetworkContiguousSlotReady(kGameState.logic_updates);
    const bool recent_starvation =
        (frame - kGameState.choke_frame) < (kGameState.framerate * 5);
    int advance = (frame_queue > 0) +
                  (!recent_starvation * (frame_queue > NetworkQueueGoal()));
    const bool is_starvation = (frame_queue == 0);
    kGameState.choke_frame =
        MAX(recent_starvation * kGameState.choke_frame, is_starvation * frame);
    for (; advance > 0; --advance) {
      uint64_t slot = NETQUEUE_SLOT(kGameState.logic_updates);
      if (ALAN) {
        assert(SlotReady(slot));
        printf(
            "Simulation "
            "[ frame %lu ] "
            "[ slot %lu ] "
            "[ advance %d ] "
            "[ jerk %lu ] "
            "[ server_jerk %lu ] "
            "[ egress_min %lu ] "
            "[ egress_max %lu ] "
            "[ queue_goal %lu ] "
            "[ ready_count %d ] "
            "\n",
            kGameState.logic_updates, slot, advance, kGameState.game_clock.jerk,
            kNetworkState.server_jerk, kNetworkState.egress_min,
            kNetworkState.egress_max, NetworkQueueGoal(),
            NetworkContiguousSlotReady(kGameState.logic_updates));
      }

      simulation::Hash();
      simulation::CacheSyncHashes(slot == 0, kGameState.logic_updates);

      // Game Mutation: Apply player commands for turn N
      InputBuffer* game_turn = GetSlot(slot);
      for (int i = 0; i < MAX_PLAYER; ++i) {
        imui::ResetTag(i);
        InputBuffer* player_turn = &game_turn[i];
        simulation::ProcessSimulation(i, player_turn->used_input_event,
                                      player_turn->input_event);
      }

      // Game Mutation: continue simulation
      simulation::Update();
#ifndef HEADLESS
      for (int i = 0; i < kNetworkState.num_players; ++i) {
        // Misc debug/feedback
        const v2f dims =
            v2f(kPlayer[i].camera.viewport.x, kPlayer[i].camera.viewport.y);
        Player* player = &kPlayer[i];
        simulation::LogPanel(dims, i);
        simulation::AdminPanel(dims, i, player);
        simulation::GameUI(dims, i, i, player);
      }
#endif

      // SetView for the local player's camera
      camera::SetView(GetCamera(kNetworkState.player_index),
                      &rgg::GetObserver()->view);

      // Give the user an update tick. The engine runs with
      // a fixed delta so no need to provide a delta time.
      ++kGameState.logic_updates;
    }

#ifndef HEADLESS
    simulation::ReadOnlyPanel(
        window::GetWindowSize(),
        imui::kEveryoneTag, kGameStats, kGameState.frame_target_usec,
        kGameState.logic_updates, kGameState.game_clock.jerk, frame_queue);
    simulation::ReadOnlyUnits(
        window::GetWindowSize(),
        imui::kEveryoneTag);

    gfx::Render(kNetworkState.player_index);
#endif

    // Capture frame time before the potential stall on vertical sync
    const uint64_t elapsed_usec = clock_delta_usec(&kGameState.game_clock);
    kGameState.frame_time_usec = elapsed_usec;
    StatsAdd(elapsed_usec, &kGameStats);

#ifndef HEADLESS
    window::SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

    if (ALAN) {
      if (kNetworkState.ack_sequence + .5f * StatsMean(&kNetworkStats) >
          kGameState.game_updates) {
        printf(
            "CONSIDER CHOKE "
            "[ ack_sequence %lu ] "
            "[ net_ftt %02.0f ] "
            "[ frame %lu] "
            "\n",
            kNetworkState.ack_sequence, StatsMean(&kNetworkStats),
            kGameState.game_updates);
      }
    }
    if (ALAN) {
      printf(
          "Update "
          "[ frame %lu ] "
          "[ choke_frame %lu ] "
          "[ recent_starvation %lu ] "
          "[ advance %lu ] "
          "\n",
          kGameState.game_updates, kGameState.choke_frame, recent_starvation,
          advance);
    }
    ++kGameState.game_updates;

    uint64_t sleep_usec = 0;
    uint64_t sleep_count = kGameState.sleep_on_loop;
    while (!clock_sync(&kGameState.game_clock, &sleep_usec)) {
      while (sleep_count) {
        --sleep_count;
        platform::sleep_usec(sleep_usec);
      };
    }
  }
  printf(
      "Exiting "
      "[ frame %d ] "
      "[ kNetworkExit %lu ] "
      "\n",
      frame, kNetworkExit);

  return 0;
}
