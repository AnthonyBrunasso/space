
#include "camera.cc"
#include "simulation.cc"

#include "gfx/gfx.cc"
#include "network/network.cc"

namespace simulation
{
static uint64_t kInputHash = DJB2_CONST;

bool
ShipFtlReady()
{
  return kShip[0].sys_engine > .5f && kShip[0].mineral >= kFtlCost;
}

void
ControlShipFtl()
{
  LOG("Faster Than Light engine activated!");
  kShip[0].ftl_frame = 1;
}

uint64_t
SelectUnit(v3f world)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    Unit* unit = &kUnit[i];

    if (v3fDsq(unit->transform.position, world) < kDsqSelect) {
      return i;
    }
  }

  return kMaxUnit;
}

void
ControlUnit(uint64_t unit_id)
{
  for (int i = 0; i < kUsedUnit; ++i) {
    bool is_unit = i == unit_id;
    kUnit[i].kind = !is_unit * (i + 1);
  }
}

unsigned
UnitIndex()
{
  for (int i = 0; i < kUsedUnit; ++i) {
    if (!kUnit[i].kind) return i;
  }
  return kMaxUnit;
}

void
DebugPanel(const v3f& my_mouse, const Stats& stats, uint64_t frame_target_usec)
{
  auto sz = window::GetWindowSize();
#define BUFFER_SIZE 64
  char buffer[BUFFER_SIZE];
  static bool enable_debug = false;
  imui::PaneOptions options;
  imui::Begin(v2f(3.f, sz.y - 30.f), options);
  imui::TextOptions debug_options;
  debug_options.color = gfx::kWhite;
  debug_options.highlight_color = gfx::kRed;
  if (imui::Text("Debug Menu", debug_options).clicked) {
    enable_debug = !enable_debug;
  }

  if (enable_debug) {
    snprintf(buffer, BUFFER_SIZE, "Frame Time: %04.02f us [%02.02f%%]",
             StatsMean(&stats), 100.f * StatsUnbiasedRsDev(&stats));
    imui::Indent(2);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Network Rtt: %06lu us [%lu/%lu queue]",
             kNetworkState.last_egress * frame_target_usec, NetworkReadyCount(),
             MAX_NETQUEUE);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Window Size: %ix%i", (int)sz.x, (int)sz.y);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Mouse Pos In World: (%.1f,%.1f)", my_mouse.x,
             my_mouse.y);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Input hash: 0x%lx", kInputHash);
    imui::Text(buffer);
    snprintf(buffer, BUFFER_SIZE, "Sim hash: 0x%lx", kSimulationHash);
    imui::Text(buffer);
    const char* ui_err = imui::LastErrorString();
    if (ui_err) imui::Text(ui_err);
    imui::Indent(-2);
  }

  static bool enable_game_menu = false;
  if (imui::Text("Game Menu", debug_options).clicked) {
    enable_game_menu = !enable_game_menu;
  }

  if (enable_game_menu) {
    imui::Indent(2);
    if (imui::Text("Reset Game", debug_options).clicked) {
      Reset();
    }
    static bool scenario_menu = false;
    if (imui::Text("Scenario", debug_options).clicked) {
      scenario_menu = !scenario_menu;
    }
    if (scenario_menu) {
      imui::Indent(2);
      if (imui::Text("Game", debug_options).clicked) {
        kScenario.type = Scenario::kGameScenario;
        Reset();
      }
      if (imui::Text("Empty", debug_options).clicked) {
        kScenario.type = Scenario::kEmptyScenario;
        Reset();
      }
      if (imui::Text("AI", debug_options).clicked) {
        kScenario.type = Scenario::kAIScenario;
        Reset();
      }
      imui::Indent(-2);
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
    if (imui::Button(math::Rect(10, 100, 40, 40), v4f(1.0f, 0.0f, 1.0f, 0.75f))
            .clicked) {
      simulation::ControlShipFtl();
    }
  }

  imui::End();
}

void
LogPanel()
{
  v2f pos;
  PlatformButton b;
  if (imui::GetUIClick(&pos, &b)) {
    LOGFMT("ui click [%d] event pos (%.2f, %.2f)", b, pos.x, pos.y);
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
}

void
ControlEvent(const PlatformEvent* event, const Camera* camera, v3f* translation)
{
  djb2_hash_more((const uint8_t*)event, sizeof(PlatformEvent), &kInputHash);

  switch (event->type) {
    case MOUSE_WHEEL: {
      // TODO(abrunasso): Why does this need to be negative?
      translation->z = -0.1f * event->wheel_delta;
    } break;
    case MOUSE_DOWN: {
      imui::MouseClick(event->position, event->button);
      v3f pos = camera::ScreenToWorldSpace(
          camera, v3f(event->position - window::GetWindowSize() * 0.5f));

      if (event->button == BUTTON_LEFT) {
        uint64_t unit = SelectUnit(pos);
        ControlUnit(unit);
      } else if (event->button == BUTTON_RIGHT) {
        uint64_t unit = UnitIndex();
        LOGFMT("Order move [%lu]", unit);
        if (unit < kUsedUnit) PushCommand({kUaMove, pos, UnitIndex()});
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

}  // namespace simulation
