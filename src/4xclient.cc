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
#include "util/cooldown.cc"

#include "4x/client_connection.cc"
#include "4x/sim.cc"
#include "4x/server.cc"

ABSL_FLAG(std::string, address, "127.0.0.1:3878",
          "If unspecified will spawn the server locally - otherwise this is "
          "assumed a multiplayer game.");

ABSL_FLAG(std::string, name, "Anthony", "Name to join server with.");

ABSL_FLAG(u64, wwidth, 1920, "Width of window.");

ABSL_FLAG(u64, wheight, 1080, "Width of window.");

ABSL_FLAG(u64, wx, UINT64_MAX, "X position of where the window starts.");

ABSL_FLAG(u64, wy, UINT64_MAX, "Y position of where the window starts.");

struct State {
  // Game and render updates per second
  u64 framerate = 60;
  // Number of times per second to sync off the network
  u64 net_syncs_per_second = 3;
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
  // How often to sync the simulation off the network.
  util::FrameCooldown net_sync_cooldown;
};

struct Interaction {
  s32 selected_unit_id = fourx::kInvalidUnit;
};

static State kGameState;
static Stats kGameStats;

static Interaction kInteraction;

static v2i kHighlighted;

static std::thread* kServerThread;

static s32 kLocalPlayerId = 0;

#define UIBUFFER_SIZE 64
static char kUIBuffer[UIBUFFER_SIZE];

constexpr u32 kHexMapSize = 10;

fourx::proto::SimulationStepRequest
CreateStepRequest()
{
  fourx::proto::SimulationStepRequest step;
  step.set_player_id(kLocalPlayerId);
  return step;
}

bool
IsSinglePlayer()
{
  static bool is_single =
      absl::GetFlag(FLAGS_address).find("127.0.0.1") != std::string::npos;
  return is_single;
}

void
MaybeStartGameServer()
{
  if (kServerThread) {
    printf("Game server already spawned.\n");
    return;
  }

  std::string address = absl::GetFlag(FLAGS_address);
  if (IsSinglePlayer()) {
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
SimUI()
{
  v2f screen = window::GetWindowSize();
  static b8 enable_players_ui = true;
  static v2f diagnostics_pos(screen.x - 415.f, screen.y);
  imui::PaneOptions options;
  options.width = options.max_width = 415.f;
  imui::Begin("Players", imui::kEveryoneTag, options, &diagnostics_pos,
              &enable_players_ui);
  std::vector<const fourx::Player*> players = fourx::SimGetPlayers();
  for (const fourx::Player* player : players) {
    imui::SameLine();
    if (player == fourx::SimActivePlayer()) {
      imui::TextOptions toptions;
      toptions.color = rgg::kGreen;
      imui::Text(std::to_string(player->id).c_str(), toptions);
    } else {
      imui::TextOptions toptions;
      toptions.color = rgg::kRed;
      imui::Text(std::to_string(player->id).c_str(), toptions);
    }
    imui::Text(": ");
    imui::Text(player->name.c_str());
    imui::NewLine();
  }
  if (!fourx::SimIsGameStarted()) {
    imui::Space(imui::kVertical, 10.f);
    imui::TextOptions toptions;
    toptions.color = rgg::kGreen;
    toptions.highlight_color = rgg::kRed;
    if (imui::Text("Start Game", toptions).clicked) {
      fourx::proto::SimulationStepRequest step_request;
      step_request.mutable_sim_start();
      fourx::ClientPushStepRequest(step_request);
    }
  }
  imui::End();
}

void
ControlUI()
{
  v2f screen = window::GetWindowSize();
  static b8 enable_control_ui = true;
  static v2f control_pos(screen.x - 415.f, 115.f);
  imui::PaneOptions options;
  options.width = options.max_width = 415.f;
  options.height = 315.f;
  imui::Begin("Control", imui::kEveryoneTag, options, &control_pos, &enable_control_ui);
  imui::TextOptions toptions;
  toptions.highlight_color = rgg::kRed;
  if (imui::Text("Create City", toptions).clicked) {
      fourx::proto::SimulationStepRequest step_request;
      step_request.set_player_id(kLocalPlayerId);
      fourx::proto::CityCreate* city_create = step_request.mutable_city_create();
      city_create->set_player_id(kLocalPlayerId);
      fourx::SimExecute(step_request);
      fourx::ClientPushStepRequest(step_request);
  }
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
InitializeGame()
{
  // Send out join game request.
  fourx::proto::PlayerJoin join;
  join.set_name(absl::GetFlag(FLAGS_name));
  fourx::proto::SimulationStepRequest step_request;
  *step_request.mutable_player_join() = join;
  fourx::ClientPushStepRequest(step_request);

  kGameState.net_sync_cooldown.frame =
      kGameState.framerate / kGameState.net_syncs_per_second;
  FrameCooldownInitialize(&kGameState.net_sync_cooldown);
}

void
PollAndExecuteNetworkEvents()
{
  fourx::proto::SimulationStepResponse step_response;
  // Handle specific step responses. Not all step requests return responses.
  // Ones that do mean the server had something specific to inform the client
  // before a sync request. Player join is one of those requests since the
  // server has authoritative knowledge of all connected clients it is the
  // only one that can assign a unique player id. That player id is needed
  // before syncs can happen.
  while (fourx::ClientPopStepResponse(&step_response)) {
    if (step_response.has_player_join_response()) {
      // printf("Setting local player id: %i\n", step_response.player_join_response().player_id());
      kLocalPlayerId = step_response.player_join_response().player_id();
    }
  }

  if (kLocalPlayerId && util::FrameCooldownReady(&kGameState.net_sync_cooldown)) {
    fourx::SimulationSyncRequest sync;
    sync.set_player_id(kLocalPlayerId);
    fourx::ClientPushSyncRequest(sync);
    util::FrameCooldownReset(&kGameState.net_sync_cooldown);
  }

  fourx::SimulationSyncResponse response;
  while (fourx::ClientPopSyncResponse(&response)) {
    if (response.steps_size() > 0) {
      for (const auto& step : response.steps()) {
        fourx::SimExecute(step);
      }
    }
  }
}

void
InitializeGameOncePlayerIdEstablished()
{
  static b8 kInitializeOnce = false;
  if (!kLocalPlayerId) return;
  if (kInitializeOnce) return;

  // Leaving out a player id will force this client to handle updating the
  // simulation when it receives its response from the server.
  {
    fourx::proto::SimulationStepRequest step_request;
    step_request.mutable_map_create()->set_size(10);
    fourx::ClientPushStepRequest(step_request);
  }

  // Spawn one unit for the player - this is probably the settler.
  {
    fourx::proto::SimulationStepRequest step_request;
    fourx::proto::UnitCreate* unit_create = step_request.mutable_unit_create();
    unit_create->set_player_id(kLocalPlayerId);
    unit_create->set_grid_x(0);
    unit_create->set_grid_y(0);
    fourx::ClientPushStepRequest(step_request);
  }

  // Single player games start immediately
  if (IsSinglePlayer()) {
    fourx::proto::SimulationStepRequest step_request;
    step_request.mutable_sim_start();
    fourx::ClientPushStepRequest(step_request);
  }

  kInitializeOnce = true;
}

void
SelectInactiveUnit(const fourx::Player* player)
{
  for (const fourx::Unit& unit : fourx::SimGetUnits()) {
    if (unit.action_points <= 0) continue;
    if (unit.player_id != player->id) continue;
    kInteraction.selected_unit_id = unit.id;
    printf("[CLIENT] Unit %i selected\n", kInteraction.selected_unit_id);
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

  fourx::Player* player = fourx::SimPlayer(kLocalPlayerId);
  if (fourx::SimActivePlayer() != player) return;
  if (!kInteraction.selected_unit_id) SelectInactiveUnit(player);

  if (mouse_down && kInteraction.selected_unit_id) {
    fourx::Unit* unit = fourx::SimUnit(kInteraction.selected_unit_id);
    std::vector<fourx::HexTile*> tiles =
        fourx::SimGetMap()->Bfs(unit->grid_pos, 1);
    for (const auto* t : tiles) {
      v2f tworld = math::HexAxialToWorld(t->grid_pos, 5.f);
      if (kHighlighted == t->grid_pos) {
        fourx::proto::SimulationStepRequest step_request;
        step_request.set_player_id(player->id);
        fourx::proto::UnitMove* move = step_request.mutable_unit_move();
        move->set_unit_id(kInteraction.selected_unit_id);
        move->set_to_grid_x(kHighlighted.x);
        move->set_to_grid_y(kHighlighted.y);
        fourx::SimExecute(step_request);
        fourx::ClientPushStepRequest(step_request);
      }
    }
  }
}

void
RenderGame(const v2f& cursor, rgg::Camera& camera, fourx::HexMap* hex_map)
{
  if (!hex_map) return;

  rgg::GetObserver()->view = camera.View();

  
  for (const auto& t : hex_map->tiles()) {
    RenderHexGridCoord(*hex_map, t.grid_pos);
  }
  
  const auto& units = fourx::SimGetUnits();
  for (const auto& u : units) {
    v2f world = math::HexAxialToWorld(u.grid_pos, 5.f);
    v4f color = rgg::kRed;
    if (u.player_id != 1) {
      color = rgg::kBlue;
    }
    rgg::RenderCube(Cubef(v3f(world, -48.5f), 2.5f, 2.5f, 2.5f), color);
  }

  const auto& cities = fourx::SimGetCities();
  for (const auto& c : cities) {
    v2f world = math::HexAxialToWorld(c.grid_pos, 5.f);
    v4f color = rgg::kRed;
    if (c.player_id != 1) {
      color = rgg::kBlue;
    }
    rgg::RenderCube(Cubef(v3f(world + v2f(1.f, 2.f), -48.5f), 1.5f, 1.5f, 5.5f), color);
    rgg::RenderCube(Cubef(v3f(world + v2f(-1.f, 1.f), -48.5f), 1.5f, 1.5f, 3.75f), color);
    rgg::RenderCube(Cubef(v3f(world + v2f(-.5f, -1.f), -48.5f), .5f, .5f, .5f), color);
    rgg::RenderCube(Cubef(v3f(world + v2f(.5f, -.75f), -48.5f), .5f, .5f, .5f), color);
    rgg::RenderCube(Cubef(v3f(world + v2f(-1.f, -1.75f), -48.5f), .5f, .5f, .5f), color);
    rgg::RenderCube(Cubef(v3f(world + v2f(1.f, -1.85f), -48.5f), .5f, .5f, .5f), color);
  }

  v2f world = math::HexAxialToWorld(kHighlighted, 5.f);
  rgg::RenderLineHexagon(v3f(world, -49.5f), 5.f, rgg::kRed);
  rgg::RenderLineHexagon(v3f(world, -49.0f), 5.f, rgg::kRed);
  rgg::RenderLineHexagon(v3f(world, -48.5f), 5.f, rgg::kRed);

  fourx::Player* player = fourx::SimPlayer(kLocalPlayerId);
  if (fourx::SimActivePlayer() == player) {
    for (const fourx::Unit& unit : fourx::SimGetUnits()) {
      if (unit.action_points <= 0) continue;
      if (unit.player_id == player->id) {
        std::vector<fourx::HexTile*> tiles =
            fourx::SimGetMap()->Bfs(unit.grid_pos, 1);
        for (const auto* t : tiles) {
          v2f tworld = math::HexAxialToWorld(t->grid_pos, 5.f);
          rgg::RenderLineHexagon(v3f(tworld, -49.5f), 5.f, rgg::kGreen);
        }
      }
    }
  }

  DebugUI(*hex_map);
  SimUI();
  ControlUI();

  rgg::DebugRenderPrimitives();
  imui::Render(imui::kEveryoneTag);
  window::SwapBuffers();
}

s32
main(s32 argc, char** argv)
{
  absl::ParseCommandLine(argc, argv);

  MaybeStartGameServer();

  fourx::ClientStartConnection(absl::GetFlag(FLAGS_address));

  platform::Clock game_clock;

  if (!memory::Initialize(MiB(64))) {
    return 1;
  }
  kGameState.window_create_info.window_width = absl::GetFlag(FLAGS_wwidth);
  kGameState.window_create_info.window_height = absl::GetFlag(FLAGS_wheight);
  kGameState.window_create_info.window_pos_x = absl::GetFlag(FLAGS_wx);
  kGameState.window_create_info.window_pos_y = absl::GetFlag(FLAGS_wy);
  if (!window::Create("Game", kGameState.window_create_info)) {
    return 1;
  }

  if (!rgg::Initialize()) {
    return 1;
  }

  InitializeGame();

  // Reset State
  StatsInit(&kGameStats);
  kGameState.game_updates = 0;
  kGameState.frame_target_usec = 1000.f * 1000.f / kGameState.framerate;
  printf("Client target usec %lu\n", kGameState.frame_target_usec);

  rgg::Camera camera(v3f(0.f, -30.f, 60.f), v3f(0.f, 1.f, 0.f));
  camera.PitchYawDelta(15.f, 0.f);

  rgg::GetObserver()->projection =
      rgg::DefaultPerspective(window::GetWindowSize());

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
              fourx::ClientStopConnection();
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

    PollAndExecuteNetworkEvents();

    InitializeGameOncePlayerIdEstablished();
   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const v2f cursor = window::GetCursorPosition();
    imui::MousePosition(cursor, imui::kEveryoneTag);

    v3f p = camera.RayFromScreenToWorld(cursor, window::GetWindowSize(), 50.f);
    v2i picked = HexWorldToAxial(p.xy(), 5.f);
    kHighlighted = picked;

    UpdateGame(cursor, mouse_down, mouse_start, camera);
    RenderGame(cursor, camera, fourx::SimGetMap());

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
    util::FrameCooldownUpdate();
  }

  return 0;
}
