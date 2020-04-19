
#include "camera.cc"
#include "simulation.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"

namespace simulation
{
#define UIBUFFER_SIZE 64
static char ui_buffer[UIBUFFER_SIZE];
static uint64_t kInputHash = DJB2_CONST;
static uint64_t kDebugInputHash;
static uint64_t kDebugSimulationHash;
static float kCameraSpeed = 4.f;

void
CacheSyncHashes(bool update, uint64_t frame)
{
  kDebugInputHash ^= (update * simulation::kInputHash);
  kDebugSimulationHash ^= (update * simulation::kSimulationHash);
#ifdef DEBUG_SYNC
  printf(
      "[Frame %u] [DebugInputHash 0x%016lx] [DebugSimulationHash 0x016%lx]\n",
      frame, kDebugInputHash, kDebugSimulationHash);
#endif
}

void
RenderBlackboard(const Unit* unit)
{
  for (int i = 0; i < kUnitBbEntryMax; ++i) {
    UnitBbEntry bb_entry = (UnitBbEntry)i;
    switch (bb_entry) {
      case kUnitDestination: {
        const Tile* t = nullptr;
        if (!BB_GET(unit->bb, kUnitDestination, t)) continue;
        snprintf(ui_buffer, sizeof(ui_buffer), "dest: %u,%u", t->cx, t->cy);
        imui::Text(ui_buffer);
      } break;
      case kUnitAttackDestination: {
        const Tile* t = nullptr;
        if (!BB_GET(unit->bb, kUnitAttackDestination, t)) continue;
        snprintf(ui_buffer, sizeof(ui_buffer), "adest: %u,%u", t->cx, t->cy);
        imui::Text(ui_buffer);
      } break;
      case kUnitTarget: {
        const uint32_t* t = nullptr;
        if (!BB_GET(unit->bb, kUnitTarget, t)) continue;
        snprintf(ui_buffer, sizeof(ui_buffer), "target: %i", *t);
        imui::Text(ui_buffer);
      } break;
      case kUnitBehavior: {
        const uint32_t* b = nullptr;
        if (!BB_GET(unit->bb, kUnitBehavior, b)) continue;
        snprintf(ui_buffer, sizeof(ui_buffer), "behavior: %i", *b);
        imui::Text(ui_buffer);
      } break;
      case kUnitAttacker: {
        const int* t = nullptr;
        if (!BB_GET(unit->bb, kUnitAttacker, t)) continue;
        snprintf(ui_buffer, sizeof(ui_buffer), "attacker: %i", *t);
        imui::Text(ui_buffer);
      } break;
      case kUnitTimer: {
        const int* t = nullptr;
        if (!BB_GET(unit->bb, kUnitTimer, t)) continue;
        snprintf(ui_buffer, sizeof(ui_buffer), "timer: %i", *t);
        imui::Text(ui_buffer);
      } break;
      default: {
        snprintf(ui_buffer, sizeof(ui_buffer), "set: %i", i);
        if (BB_EXI(unit->bb, i)) {
          imui::Text(ui_buffer);
        }
      }
    }
  }
}

void
ReadOnlyPanel(v2f screen, uint32_t tag, const Stats& stats,
              uint64_t frame_target_usec, uint64_t frame, uint64_t jerk,
              uint64_t frame_queue)
{
  static bool enable_debug = false;
  static v2f read_only_pos(3.f, screen.y);
  static float right_align = 155.f;
  imui::PaneOptions options;
  imui::Begin("Diagnostics Debug", tag, options, &read_only_pos, &enable_debug);
  imui::TextOptions debug_options;
  debug_options.color = gfx::kWhite;
  debug_options.highlight_color = gfx::kRed;
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Frame Time");
  snprintf(ui_buffer, sizeof(ui_buffer),
           "%04.02f us [%02.02f%%] [%lu jerk] [%lu server_jerk]",
           StatsMean(&stats), 100.f * StatsUnbiasedRsDev(&stats), jerk,
           kNetworkState.server_jerk);
  imui::Text(ui_buffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Network Rtt");
  snprintf(ui_buffer, sizeof(ui_buffer),
           "[%06lu us to %06lu us] [%lu/%lu queue]",
           kNetworkState.egress_min * frame_target_usec,
           kNetworkState.egress_max * frame_target_usec, frame_queue,
           MAX_NETQUEUE);
  imui::Text(ui_buffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Network ft");
  snprintf(ui_buffer, sizeof(ui_buffer), "Network ft: %04.02f mean [%02.02f%%]",
           StatsMean(&kNetworkStats),
           100.f * StatsUnbiasedRsDev(&kNetworkStats));
  imui::Text(ui_buffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Network rsdev");
  snprintf(ui_buffer, sizeof(ui_buffer),
           "[%04.02f 84th] [%04.02f 97th ] [%04.02f 99th]",
           StatsRsDev(&kNetworkStats) * 1, StatsRsDev(&kNetworkStats) * 2,
           StatsRsDev(&kNetworkStats) * 3);
  imui::Text(ui_buffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Network Queue");
  snprintf(ui_buffer, sizeof(ui_buffer), "%lu [%1.0fx rsdev]",
           NetworkQueueGoal(), kNetworkState.rsdev_const);
  imui::Text(ui_buffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Window Size");
  snprintf(ui_buffer, sizeof(ui_buffer), "%04.0fx%04.0f", screen.x, screen.y);
  imui::Text(ui_buffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Input hash");
  snprintf(ui_buffer, sizeof(ui_buffer), "0x%lx", kDebugInputHash);
  imui::Text(ui_buffer);
  imui::NewLine();
  imui::SameLine();
  imui::Width(right_align);
  imui::Text("Sim hash");
  snprintf(ui_buffer, sizeof(ui_buffer), "0x%lx", kDebugSimulationHash);
  imui::Text(ui_buffer);
  imui::NewLine();
  imui::SameLine();
  const char* ui_err = imui::LastErrorString();
  if (ui_err) imui::Text(ui_err);
  imui::End();

#if 1
  static v2f pos(3.f, screen.y - 300.f);
  static bool show = false;
  imui::DebugPane("Diagnostics UI", tag, &pos, &show);
#endif
}

bool
EntityView(int idx, Player* player, uint32_t player_index,
           bool show_modules, bool show_units, bool show_selected)
{
  Entity* entity = &kEntity[idx];
  Unit* unit = i2Unit(idx);
  Module* module = i2Module(idx);
  if (module && !show_modules) return false;
  if (unit && !show_units) return false;
  if (show_selected && (entity->control & (1 << player_index)) == 0)
    return false;
  const char* entity_type = unit ? "Unit" : module ? "Module" : "Unknown";
  snprintf(ui_buffer, sizeof(ui_buffer), "%s %d", entity_type, entity->id);
  imui::TextOptions toptions;
  toptions.highlight_color = gfx::kRed;
  imui::Result imresult = imui::Text(ui_buffer, toptions);
  if (imresult.clicked) {
    camera::Move(&player->camera, entity->position);
  }
  imui::Indent(2);
  if (entity) {
    snprintf(ui_buffer, sizeof(ui_buffer), "tile %u %u", entity->tile.cx,
             entity->tile.cy);
    imui::Text(ui_buffer);
    snprintf(ui_buffer, sizeof(ui_buffer), "position %04.0f %04.0f",
             entity->position.x, entity->position.y);
    imui::Text(ui_buffer);

  }
  if (show_units && unit) {
    snprintf(ui_buffer, sizeof(ui_buffer), "action %d",
             unit->uaction);
    imui::Text(ui_buffer);
    snprintf(ui_buffer, sizeof(ui_buffer), "persistent_action %d",
             unit->persistent_uaction);
    imui::Text(ui_buffer);
    RenderBlackboard(unit);
  }
  if (show_modules && module) {
    snprintf(ui_buffer, sizeof(ui_buffer), "mkind %s", ModuleName(module->mkind));
    imui::Text(ui_buffer);
#if 0
    // Not super useful right now.
    imui::SameLine();
    imui::Text("build");
    imui::ProgressBar(170.f, 5.f, module->frames_building, module->frames_to_build,
                      v4f(0.f, 1.f, 0.f, .7f), v4f(.2f, .2f, .2f, 1.f));
    imui::NewLine();
    imui::SameLine();
    imui::Text("train");
    imui::ProgressBar(170.f, 5.f, module->frames_training, module->frames_to_train,
                      v4f(0.f, 1.f, 0.f, .7f), v4f(.2f, .2f, .2f, 1.f));
    imui::NewLine();
#endif
  }
  imui::Indent(-2);
  return imresult.highlighted;
}

void
EntityViewer(v2f screen, uint32_t tag, Player* player)
{
  static bool unit_debug = false;
  static v2f unit_debug_pos(screen.x - 300.f, screen.y);
  imui::PaneOptions options;
  options.width = 300.f;
  options.max_width = 300.f;
  options.max_height = 700.f;
  imui::Begin("Entity Viewer", tag, options, &unit_debug_pos, &unit_debug);
  snprintf(ui_buffer, sizeof(ui_buffer), "Entity Capacity (%u / %u)",
           kUsedEntity, kMaxEntity);
  imui::Text(ui_buffer);
  imui::Space(imui::kVertical, 1.f);
  imui::ProgressBar(200.f, 8.f, kUsedEntity, kMaxEntity, gfx::kRed,
                    v4f(.3f, .3f, .3f, 1.f));
  imui::Space(imui::kVertical, 5.f);
  imui::Text("Filter");
  imui::Indent(2);
  imui::SameLine();
  static bool show_modules = true;
  imui::Width(100.f);
  imui::Text("Modules");
  imui::Checkbox(16.f, 16.f, &show_modules);
  imui::NewLine();
  imui::SameLine();
  static bool show_units = true;
  imui::Width(100.f);
  imui::Text("Units");
  imui::Checkbox(16.f, 16.f, &show_units);
  imui::NewLine();
  imui::SameLine();
  static bool show_selected = false;
  imui::Width(100.f);
  imui::Text("Selected");
  imui::Checkbox(16.f, 16.f, &show_selected);
  imui::NewLine();
  imui::Indent(-2);
  for (int i = 0; i < kUsedEntity; ++i) {
    // Draws a red line cube around the entity.
    // NOTE: assuming tag == player_index
    if (EntityView(i, player, tag, show_modules, show_units, show_selected)) {
      gfx::PushDebugCube(
          Cubef(
              kEntity[i].position + v3f(0.f, 0.f, kEntity[i].bounds.z / 2.f),
              kEntity[i].bounds),
          gfx::kRed);
    }
  }
  imui::End();
}

void
TilePanel(v2f screen, uint32_t tag, Player* player)
{
  imui::PaneOptions options;
  options.width = 300.f;
  imui::Begin("Tile Debug", tag, options, &player->tile_menu_pos,
              &player->tile_menu);
  imui::TextOptions debug_options;
  debug_options.color = gfx::kWhite;
  debug_options.highlight_color = gfx::kRed;
  Tile tile = kZeroTile;
  if (TileValid(player->mouse_tile)) {
    tile = player->mouse_tile;
  }
  snprintf(ui_buffer, sizeof(ui_buffer), "%u X %u Y", tile.cx, tile.cy);
  imui::Text(ui_buffer, debug_options);
  snprintf(ui_buffer, sizeof(ui_buffer), "blocked %u", tile.blocked);
  imui::Text(ui_buffer, debug_options);
  snprintf(ui_buffer, sizeof(ui_buffer), "nooxygen %u", tile.nooxygen);
  imui::Text(ui_buffer, debug_options);
  snprintf(ui_buffer, sizeof(ui_buffer), "shroud %u", tile.shroud);
  imui::Text(ui_buffer, debug_options);
  snprintf(ui_buffer, sizeof(ui_buffer), "visible %u", tile.visible);
  imui::Text(ui_buffer, debug_options);
  snprintf(ui_buffer, sizeof(ui_buffer), "exterior %u", tile.exterior);
  imui::Text(ui_buffer, debug_options);
  snprintf(ui_buffer, sizeof(ui_buffer), "explored %u", tile.explored);
  imui::Text(ui_buffer, debug_options);
  if (player->tile_menu) {
    rgg::RenderRectangle(FromShip(tile).Center(), gfx::kTileScale,
                         gfx::kDefaultRotation, gfx::kGray);
  }
  imui::SameLine();
  imui::Text("flags ");
  imui::Bitfield16(tile.flags);
  imui::NewLine();
  imui::End();
}

void
AdminPanel(v2f screen, uint32_t tag, Player* player)
{
  imui::PaneOptions options;
  options.width = 300.f;
  imui::Begin("Admin Menu", tag, options, &player->admin_menu_pos,
              &player->admin_menu);
  imui::TextOptions text_options;
  text_options.color = gfx::kWhite;
  text_options.highlight_color = gfx::kRed;
  imui::SameLine();
  imui::Width(160.f);
  imui::Text("Render Grid");
  imui::Checkbox(16.f, 16.f, &gfx::kRenderGrid);
  imui::NewLine();
  imui::SameLine();
  imui::Width(160.f);
  imui::Text("Render Path");
  imui::Checkbox(16.f, 16.f, &gfx::kRenderPath);
  imui::NewLine();
  imui::SameLine();
  imui::Width(160.f);
  imui::Text("Render Cam Tgt");
  imui::Checkbox(16.f, 16.f, &camera::kShowCameraDebugTarget);
  imui::NewLine();
  imui::SameLine();
  imui::Width(160.f);
  imui::Text("Mineral Cheat");
  imui::Checkbox(16.f, 16.f, &player->mineral_cheat);
  imui::NewLine();
  if (imui::Text("Spawn Unit Cheat", text_options).clicked) {
    v2f pos = math::RandomPointInRect(ShipBounds(player - kPlayer));
    SpawnCrew(ToShip(player->ship_index, pos), player - kPlayer);
  }
  if (imui::Text("Kill Random Unit Cheat", text_options).clicked) {
    // Kill first unit in entity list.
    int rand_val = rand() % kUsedEntity;
    for (int i = 0; i < kUsedEntity; ++i) {
      uint64_t idx = (rand_val + i) % kUsedEntity;
      Unit* unit = i2Unit(idx);
      if (!unit) continue;
      LOGFMT("Kill unit %i", unit->id);
      ZeroEntity(unit);
      break;
    }
  }
  if (imui::Text("Reset Game", text_options).clicked) {
    Reset(kNetworkState.game_id);
  }
  if (imui::Text("Scenario", text_options).clicked) {
    player->scenario_menu = !player->scenario_menu;
  }
  if (player->scenario_menu) {
    imui::Indent(2);
    for (int i = 0; i < kMaxScenario; ++i) {
      if (imui::Text(kScenarioNames[i], text_options).clicked) {
        kScenario = (ScenarioType)i;
        Reset(kNetworkState.game_id);
      }
    }
    imui::Indent(-2);
  }
  if (imui::Text("Exit", text_options).clicked) {
    exit(1);
  }

  imui::End();
}

void
LogPanel(v2f screen_dims, uint32_t tag)
{
  imui::PaneOptions pane_options;
  pane_options.width = pane_options.max_width = 450.f;
  pane_options.height = pane_options.max_height = 160.f;
  pane_options.enable_console_mode = true;
  imui::TextOptions text_options;
  static v2f pos(screen_dims.x - pane_options.width, pane_options.height);
  static bool show = true;
  imui::Begin("Console Log", tag, pane_options, &pos, &show);
  for (int i = 0, imax = LogCount(); i < imax; ++i) {
    const char* log_msg = ReadLog(i);
    if (!log_msg) continue;
    imui::Text(log_msg, text_options);
  }
  imui::End();
}

void
ControlEvent(const PlatformEvent event, uint64_t player_index, Player* player)
{
  v3f event_world = camera::ScreenToWorldSpace(&player->camera, event.position);
  Tile event_tile = ToAnyShip(event_world);

  djb2_hash_more((const uint8_t*)&event, sizeof(PlatformEvent), &kInputHash);
  switch (event.type) {
    case MOUSE_POSITION: {
      player->mouse_world = event_world;
      player->mouse_tile = event_tile;
    } break;
    case MOUSE_WHEEL: {
      imui::MouseWheel(event.wheel_delta, player_index);
      if (imui::MouseInUI(player_index) ||
          imui::MouseInUI(imui::kEveryoneTag)) {
        break;
      }
      // TODO(abrunasso): Why does this need to be negative?
      player->camera.motion.z = -10.f * event.wheel_delta;
    } break;
    case MOUSE_DOWN: {
      imui::MouseDown(event.position, event.button, player_index);
      if (imui::MouseInUI(player_index) ||
          imui::MouseInUI(imui::kEveryoneTag))
        break;

      if (event.button == BUTTON_MIDDLE) {
        camera::Move(&player->camera, event_world);
      }

      if (event.button == BUTTON_LEFT) {
        switch (player->hud_mode) {
          case kHudSelection: {
            player->selection_start = event_world;
            UnselectPlayerAll(player_index);
          } break;
          case kHudAttackMove: {
            LOGFMT("Order attack move [%.0f,%.0f]", event_world.x,
                   event_world.y);
            PushCommand({kUaAttackMove, event_world, kInvalidId,
                         (unsigned)(1 << player_index)});
          } break;
          case kHudModule: {
            if (!TileValid(event_tile)) break;

            ModuleKind mkind = player->mod_placement;
            if (!ModuleCanBuild(mkind, player)) {
              LOGFMT("Player can't afford module %i", mkind);
              break;
            }
            Module* mod = UseEntityModule();
            mod->bounds = ModuleBounds(mkind);
            mod->mkind = mkind;
            mod->ship_index = player->ship_index;
            mod->player_index = player_index;
            // Rase module to have bottom touch 0,0.
            mod->position = v3f(0.f, 0.f, mod->bounds.z / 2.f) +
                            FromShip(event_tile).Center();
            player->mineral -= ModuleCost(mkind);
            LOGFMT("Order build [%i] [%i,%i]", mkind, event_tile.cx,
                   event_tile.cy);
            PushCommand({kUaBuild, event_world, kInvalidId,
                         (unsigned)(1 << player_index)});
          } break;
        }
      } else if (event.button == BUTTON_RIGHT) {
        Unit* target = GetUnitTarget(player_index, event_world);
        if (target) {
          LOGFMT("Order attack [%lu]", target->id);
          PushCommand({kUaAttack, event_world, kInvalidId,
                       (unsigned)(1 << player_index)});
        } else {
          LOGFMT("Order move [%.0f,%.0f]", event_world.x, event_world.y);
          PushCommand({kUaMove, event_world, kInvalidId,
                       (unsigned)(1 << player_index)});
        }
      }

      player->hud_mode = kHudDefault;
    } break;
    case MOUSE_UP: {
      imui::MouseUp(event.position, event.button, player_index);
      if (event.button == BUTTON_LEFT) {
        if (imui::MouseInUI(event.position, player_index) ||
            imui::MouseInUI(event.position, imui::kEveryoneTag))
          break;
        // TODO(abrunasso): Unconvinced this is the best way to check if a
        // selection occurred. Also unconvined that it's not....
        Unit* unit = nullptr;
        if (player->selection_start.x != 0.f ||
            player->selection_start.y != 0.f ||
            player->selection_start.z != 0.f) {
          v3f diff = player->mouse_world - player->selection_start;
          Rectf sbox(player->selection_start.x, player->selection_start.y,
                     diff.x, diff.y);
          sbox = math::OrientToAabb(sbox);
          bool selected = false;
          for (int i = 0; i < kUsedEntity; ++i) {
            unit = SelectUnit(sbox, i);
            if (!unit) continue;
            LOGFMT("Select unit: %i", unit->id);
            SelectPlayerUnit(player_index, unit);
            selected = true;
          }

          // Prefer selecting units over modules.
          for (int i = 0; i < kUsedEntity && !selected; ++i) {
            Module* mod = SelectModule(sbox, i);
            if (!mod) continue;
            LOGFMT("Select Module: %i", i);
            SelectPlayerModule(player_index, mod);
            selected = true;
          }
        }
        player->selection_start = v3f(0.f, 0.f, 0.f);
        // Box selection missed, fallback to single unit selection
        if (!unit) {
          unit = GetUnit(event_world);
          SelectPlayerUnit(player_index, unit);
        }
      }
    } break;
    case KEY_DOWN: {
      switch (event.key) {
        case 27 /* ESC */: {
          exit(1);
        } break;
        case 'w': {
          player->camera.motion.y = kCameraSpeed;
        } break;
        case 'a': {
          player->camera.motion.x = -kCameraSpeed;
        } break;
        case 's': {
          player->camera.motion.y = -kCameraSpeed;
        } break;
        case 'd': {
          player->camera.motion.x = kCameraSpeed;
        } break;
        case 'r': {
          // This check has to happen, otherwise the cursor will go into attack
          // move with no units selected and you won't be able to select
          // units or attack without left clicking again.
          if (CountUnitSelection(player_index) > 0) {
            player->hud_mode = kHudAttackMove;
          }
        } break;
        case ' ': {
          for (int i = 0; i < kUsedShip; ++i) {
            if (kShip[i].level != player->level) continue;
            bool ftl_ready = true;
            if (!ftl_ready) {
              LOG("Faster Than Light engine is offline!");
            } else if (player->mineral >= kFtlCost) {
              LOG("Faster Than Light engine activated!");
              kShip[i].ftl_frame = 1;
              player->mineral -= kFtlCost;
            } else {
              LOGFMT("Ftl requires minerals [%d]!", kFtlCost);
            }
          }
        } break;
        // case '1': {
        //  player->hud_mode = kHudModule;
        //  player->mod_placement = 0;
        //} break;
        // case '2': {
        //  player->hud_mode = kHudModule;
        //  player->mod_placement = 1;
        //} break;
        case '1': {
          player->hud_mode = kHudModule;
          player->mod_placement = kModMine;
        } break;
        case '2': {
          player->hud_mode = kHudModule;
          player->mod_placement = kModBarrack;
        } break;
        case '3': {
          player->hud_mode = kHudModule;
          player->mod_placement = kModMedbay;
        } break;
        case '4': {
          player->hud_mode = kHudModule;
          player->mod_placement = kModWarp;
        } break;
        case 'h': {
          player->level = CLAMP(player->level - 1, 1, 10);
        } break;
        case 'l': {
          player->level = CLAMP(player->level + 1, 1, 10);
        } break;
        case 'j': {
          if (player->ship_index < kUsedShip) {
            Ship* ship = &kShip[player->ship_index];
            ship->deck -= (ship->deck > 0);
          }
        } break;
        case 'k': {
          if (player->ship_index < kUsedShip) {
            Ship* ship = &kShip[player->ship_index];
            ship->deck += (ship->deck < kLastDeck);
          }
        } break;
        default:
          break;
      }
    } break;
    case KEY_UP: {
      switch (event.key) {
        case 'w': {
          player->camera.motion.y = 0.f;
        } break;
        case 'a': {
          player->camera.motion.x = 0.f;
        } break;
        case 's': {
          player->camera.motion.y = 0.f;
        } break;
        case 'd': {
          player->camera.motion.x = 0.f;
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
GameUI(v2f screen, uint32_t tag, int player_index, Player* player)
{
  imui::PaneOptions options;
  bool show = true;
  imui::Begin("Game UI", tag, options, &player->game_menu_pos, &show);
  imui::Result hud_result;
  v2f p(50.f, 10.f);
  for (int i = 3; i < kModCount; ++i) {
    imui::Space(imui::kVertical, 8.f);
    imui::SameLine();
    v3f c = ModuleColor((ModuleKind)i);
    hud_result = imui::Button(35.f, 35.f, v4f(c.x, c.y, c.z, .6f));
    imui::Space(imui::kHorizontal, 5.f);
    imui::Text(ModuleName((ModuleKind)i));
    p.x += 55.f;
    if (hud_result.clicked) {
      player->hud_mode = kHudModule;
      player->mod_placement = (ModuleKind)i;
    }
    imui::NewLine();
  }
  imui::Space(imui::kVertical, 2.f);
  imui::HorizontalLine(v4f(0.4f, 0.4f, 0.4f, 1.f));
  imui::Space(imui::kVertical, 13.f);
  if (player->ship_index < kUsedShip) {
    Ship* ship = &kShip[player->ship_index];
    snprintf(ui_buffer, sizeof(ui_buffer), "%.12s Deck", deck_name[ship->deck]);
    imui::Text(ui_buffer);
  }
  snprintf(ui_buffer, sizeof(ui_buffer), "Minerals: %.1f", player->mineral);
  imui::Text(ui_buffer);
  snprintf(ui_buffer, sizeof(ui_buffer), "Level: %lu", player->level);
  imui::Text(ui_buffer);
  snprintf(ui_buffer, sizeof(ui_buffer), "Player Index: %zu", player - kPlayer);
  imui::Text(ui_buffer);
  if (simulation::kSimulationOver) {
    snprintf(ui_buffer, sizeof(ui_buffer), "Game Over");
    imui::Text(ui_buffer);
  }
  imui::End();
}

void
ProcessSimulation(int player_index, uint64_t event_count,
                  const PlatformEvent* event)
{
  Player* p = &kPlayer[player_index];
  // This is done here due to MOUSE_SCROLL relying on having a mouse position
  // in UI check done. If this is not done before ControlEvent it is possible
  // to get events in such an order that camera would zoom when a user means
  // to scroll up or down on a UI pane.
  for (int i = 0; i < event_count; ++i) {
    const PlatformEvent* pevent = &event[i];
    switch (pevent->type) {
      case MOUSE_POSITION: {
        imui::MousePosition(pevent->position, player_index);
      } break;
      default: break;
    }
  }
  for (int i = 0; i < event_count; ++i) {
    ControlEvent(event[i], player_index, p);
  }
}

}  // namespace simulation
