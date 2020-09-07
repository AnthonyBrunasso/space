#include <cstdio>
#include <vector>

#include <ecs/ecs.cc>
#include <platform/platform.cc>

struct AnimFrame {

  r32 x;
  r32 y;
  r32 width;
  r32 height;
  u32 length;

};

enum AdventurerState {
  kAdventurerIdle = 0,
  kAdventurerWalk = 1,
  kAdventurerJump = 2,
  kAdventurerNumStates = 3,
};

struct FrameBuilder {
  FrameBuilder(std::vector<AnimFrame>* frames) :
    frames(frames) {}

  FrameBuilder&
  Frame(r32 x, r32 y, r32 width, r32 height, u32 length)
  {
    frames->push_back({x, y, width, height, length});
    return *this;
  }

  std::vector<AnimFrame>* frames;
};

struct FSM {
  FSM(u32 num_states) : frames(num_states) {}

  FrameBuilder
  Node(u32 state)
  {
    assert(state < frames.size());
    return FrameBuilder(&frames[state]);
  }

  void
  DebugPrint()
  {
    for (u32 i = 0; i < frames.size(); ++i) {
      printf("ANIM %i\n", i); 
      for (const auto& af : frames[i]) {
        printf("  %.2f, %.2f, %.2f, %.2f, %u\n",
               af.x, af.y, af.width, af.height, af.length);
      }
    }
  }

  std::vector<std::vector<AnimFrame>> frames;

  // Entity this FSM belongs to.
  ecs::Entity entity = 0;
};

int
main(int argc, char** argv)
{
  printf("Animate me...\n");

  FSM adventurer_fsm(kAdventurerNumStates);

  adventurer_fsm.Node(kAdventurerIdle)
      .Frame(0.f, 37.f * 0.f, 50.f, 37.f, 25)
      .Frame(0.f, 37.f * 1.f, 50.f, 37.f, 25)
      .Frame(0.f, 37.f * 2.f, 50.f, 37.f, 25)
      .Frame(0.f, 37.f * 3.f, 50.f, 37.f, 25);

  adventurer_fsm.Node(kAdventurerWalk)
      .Frame(50.f, 37.f * 1.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 2.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 3.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 4.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 5.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 6.f, 50.f, 37.f, 25);

  adventurer_fsm.Transition(
      kAdventurerIdle, kAdventurerWalk,
      [](ecs::Entity entity) {
      });

  adventurer_fsm.DebugPrint(); 

  return 0;
}
