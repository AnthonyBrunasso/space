// namespace live {

DEFINE_CALLBACK(HarvestBoxSelect, Rectf);
DEFINE_CALLBACK(ZoneBoxSelect, Rectf);
DEFINE_CALLBACK(BuildLeftClick, v2f);

struct Interaction {
  enum Action {
    kNone = 0,
    kHarvest = 1,
    kBuild = 2,
    kZone = 3,
  };

  v2f mouse_pos;

  b8 left_mouse_down = false;
  v2f left_mouse_start;
  Action action = kHarvest;

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
  toptions.color = kInteraction.action == Interaction::kHarvest ? imui::kRed : imui::kWhite;
  if (imui::Text("1. Harvest", toptions).clicked) {
    kInteraction.action = Interaction::kHarvest;
  }
  toptions.color = kInteraction.action == Interaction::kBuild ? imui::kRed : imui::kWhite;
  if (imui::Text("2. Build", toptions).clicked) {
    kInteraction.action = Interaction::kBuild;
  }
  toptions.color = kInteraction.action == Interaction::kZone ? imui::kRed : imui::kWhite;
  if (imui::Text("3. Zone", toptions).clicked) {
    kInteraction.action = Interaction::kZone;
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
        if (kInteraction.action == Interaction::kBuild) {
          DispatchBuildLeftClick(wpos);
        }
      }
      if (event.button == BUTTON_RIGHT) {
        kInteraction.action = Interaction::kHarvest;
      }
    } break;
    case MOUSE_UP: {
      if (event.button == BUTTON_LEFT) {
        v2f wpos = rgg::CameraRayFromMouseToWorld(event.position, 1.f).xy();
        kInteraction.left_mouse_down = false;
        if (kInteraction.action == Interaction::kHarvest) {
          DispatchHarvestBoxSelect(math::OrientToAabb(kInteraction.selection_rect()));
        } else if (kInteraction.action == Interaction::kZone) {
          DispatchZoneBoxSelect(math::OrientToAabb(kInteraction.selection_rect()));
        }
      }
    } break;
    case MOUSE_WHEEL:
    case KEY_DOWN: {
      switch (event.key) {
        case '1':
          kInteraction.action = Interaction::kHarvest;
          break;
        case '2':
          kInteraction.action = Interaction::kBuild;
          break;
        case '3':
          kInteraction.action = Interaction::kZone;
          break;
      }
    } break;
    case KEY_UP:
    case MOUSE_POSITION:
    case XBOX_CONTROLLER:
    case NOT_IMPLEMENTED:
    deafult: break;
  }
}

void
InteractionRender()
{
  if (kInteraction.left_mouse_down && kInteraction.action == Interaction::kHarvest) {
    Rectf srect = math::OrientToAabb(kInteraction.selection_rect());
    ECS_ITR2(itr, kPhysicsComponent, kHarvestComponent);
    while (itr.Next()) {
      PhysicsComponent* tree = itr.c.physics;
      Rectf trect = tree->rect();
      b8 irect = math::IntersectRect(trect, srect);
      b8 crect = math::IsContainedInRect(trect, srect);
      if (irect || crect) {
        Rectf render_rect = trect;
        render_rect.x -= 1.f;
        render_rect.y -= 1.f;
        render_rect.width += 2.f;
        render_rect.height += 2.f;
        rgg::DebugPushRect(render_rect, v4f(1.f, 1.f, 1.f, 1.f));
      }
    }
    rgg::DebugPushRect(srect, v4f(0.f, 1.f, 0.f, 0.8f));
  }

  if (kInteraction.left_mouse_down && kInteraction.action == Interaction::kZone) {
    Rectf srect = math::OrientToAabb(kInteraction.selection_rect());
    rgg::DebugPushRect(srect, v4f(0.f, 0.f, 1.f, 0.8f));
  }

  if (kInteraction.action == Interaction::kBuild) {
    v2f mouse_pos = rgg::CameraRayFromMouseToWorld(window::GetCursorPosition(), 1.f).xy();
    v2f grid_pos;
    if (GridClampPos(mouse_pos, &grid_pos)) {
      Rectf rect(grid_pos, v2f(kWallWidth, kWallHeight));
      rgg::DebugPushRect(rect, v4f(1.f, 1.f, 1.f, 1.f));
    }
  }
}

// }
