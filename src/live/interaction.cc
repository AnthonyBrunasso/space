// namespace live {

struct Interaction {
  b8 left_mouse_down = false;
  v2f left_mouse_start;

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
  if (imui::Text("Trees", toptions).clicked) {
    puts("hi");
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
        printf("(%.2f %.2f) (%.2f %.2f)\n",
               kInteraction.left_mouse_start.x, 
               kInteraction.left_mouse_start.y, 
               wpos.x, wpos.y);
        kInteraction.left_mouse_down = false;
      }
    } break;
    case NOT_IMPLEMENTED:
    case MOUSE_WHEEL:
    case KEY_DOWN:
    case KEY_UP:
    case MOUSE_POSITION:
    case XBOX_CONTROLLER:
    deafult: break;
  }
}

// }
