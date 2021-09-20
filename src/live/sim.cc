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
#include "live/interaction.cc"

struct Sim {
  std::vector<Tree> trees;
  std::vector<Character> characters;
};

static Sim kSim;

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
