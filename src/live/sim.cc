#pragma once


namespace live {


struct Entity {
  Entity(v2f pos, v2f bounds) : pos(pos), bounds(bounds) {}
  v2f pos;
  v2f bounds;

  Rectf rect() const { return Rectf(pos, bounds); }
};

struct Tree : public Entity {
  Tree(v2f pos) : Entity(pos, v2f(live::kTreeWidth, live::kTreeHeight)) {}
};

struct Character : public Entity {
  Character(v2f pos) : Entity(pos, v2f(live::kCharacterWidth, live::kCharacterHeight)) {}

  // Order the character should be executing.
  s32 order_id = 0;
};

#include "live/order.cc"

struct Sim {
  std::vector<Tree> trees;
  std::vector<Character> characters;
};

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

static Sim kSim;
static Interaction  kInteraction;

void
SimProcessPlatformEvent(const PlatformEvent& event)
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

void
SimInitialize()
{
  kSim.trees.push_back(Tree(v2f(0.f, 0.f)));
  kSim.trees.push_back(Tree(v2f(15.f, 8.f)));
  kSim.trees.push_back(Tree(v2f(-8.f, -12.5f)));
  kSim.trees.push_back(Tree(v2f(-4.f, 20.f)));
  kSim.characters.push_back(Character(v2f(-80.f, 100.f)));
  kSim.characters.push_back(Character(v2f(80.f, 120.f)));
}

const std::vector<Tree>&
SimTrees()
{
  return kSim.trees;
}

const std::vector<Character>&
SimCharacters()
{
  return kSim.characters;
}

void
SimUpdate()
{
  if (kInteraction.left_mouse_down) {
    Rectf srect = math::OrientToAabb(kInteraction.selection_rect());
    for (int i = 0; i < kSim.trees.size(); ++i) {
      Tree* tree = &kSim.trees[i];
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
    rgg::DebugPushRect(srect, v4f(0.f, 1.f, 0.f, 0.2f));
  }

  for (int i = 0; i < kSim.characters.size(); ++i) {
    Character* character = &kSim.characters[i];
    OrderAcquire(character);
    OrderExecute(character);
  }
}

}
