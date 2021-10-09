// namespace live {

DEFINE_CALLBACK(BoxSelect, Rectf);

struct Interaction {
  enum Action {
    NONE = 0,
    HARVEST = 1,
  };

  b8 left_mouse_down = false;
  v2f left_mouse_start;
  Action action = HARVEST;

  Rectf selection_rect()
  {
    v2f cursor = window::GetCursorPosition();
    v2f wpos = rgg::CameraRayFromMouseToWorld(cursor, 1.f).xy();
    return math::MakeRect(left_mouse_start, wpos);
  }
};

static Interaction kInteraction;

void
InteractionRenderOrderOptions()
{
  v2f screen = window::GetWindowSize();

  static b8 enable_ui = true;
  static v2f pos_ui(screen.x - 115.f, screen.y);
  imui::PaneOptions options;
  imui::Begin("Selection", imui::kEveryoneTag, options, &pos_ui, &enable_ui);
  imui::TextOptions toptions;
  toptions.highlight_color = imui::kRed;
  if (imui::Text("Harvest", toptions).clicked) {
    kInteraction.action = Interaction::HARVEST;
  }
  imui::End();
}

void
InteractionRenderResourceCounts()
{
  static char resource_buffer[64];

  v2f screen = window::GetWindowSize();

  static b8 enable_ui = true;
  static v2f pos_ui(0.f, 500.f);
  imui::PaneOptions options;
  imui::Begin("Resources", imui::kEveryoneTag, options, &pos_ui, &enable_ui);
  for (s32 i = 0; i < kResourceTypeCount; ++i) {
    snprintf(resource_buffer, sizeof(resource_buffer), "%s: %i",
             ResourceName((ResourceType)i), kSim.resources[i]);
    imui::Text(resource_buffer);
  }
  imui::End();
}

void
InteractionProcessPlatformEvent(const PlatformEvent& event)
{
  switch (event.type) {
    case MOUSE_DOWN: {
      if (event.button == BUTTON_LEFT) {
        v2f wpos = rgg::CameraRayFromMouseToWorld(event.position, 1.f).xy();
        //OrderCreateMove(wpos);
        kInteraction.left_mouse_down = true;
        kInteraction.left_mouse_start = wpos;
      }
    } break;
    case MOUSE_UP: {
      if (event.button == BUTTON_LEFT) {
        v2f wpos = rgg::CameraRayFromMouseToWorld(event.position, 1.f).xy();
        kInteraction.left_mouse_down = false;
        DispatchBoxSelect(math::OrientToAabb(kInteraction.selection_rect()));
      }
    } break;
    case MOUSE_WHEEL:
    case KEY_DOWN:
    case KEY_UP:
    case MOUSE_POSITION:
    case XBOX_CONTROLLER:
    case NOT_IMPLEMENTED:
    deafult: break;
  }
}

// }
