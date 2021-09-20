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

static Sim kSim;

void
SimProcessPlatformEvent(const PlatformEvent& event)
{
  switch (event.type) {
    case MOUSE_DOWN: {
      if (event.button == BUTTON_LEFT) {
        OrderCreateMove(rgg::CameraRayFromMouseToWorld(event.position, 1.f).xy());
      }
    } break;
    case NOT_IMPLEMENTED:
    case MOUSE_UP:
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
  for (int i = 0; i < kSim.characters.size(); ++i) {
    Character* character = &kSim.characters[i];
    OrderAcquire(character);
    OrderExecute(character);
  }
}

}
