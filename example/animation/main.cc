#include <cstdio>
#include <vector>

#include <common/common.cc>
#include <platform/platform.cc>
#include <math/math.cc>

namespace ecs {

  struct Entity {
    u32 id = 0;
  };

}

struct AnimFrame {
  r32 x;
  r32 y;
  r32 width;
  r32 height;
  u32 length;

  Rectf
  rect()
  {
    return Rectf(x, y, width, height);
  }
};

enum AdventurerState {
  kAdventurerIdle = 0,
  kAdventurerWalk = 1,
  kAdventurerJump = 2,
  kAdventurerNumStates = 3,
};

typedef std::function<b8(ecs::Entity)> TransitionFunc;

enum FSMNodeFlags {
  // If set animation will not reset to the first frame when reaching the last.
  kFSMNodeNoCycle = 0,
};

struct FSMNodeData {
  std::vector<AnimFrame> frames;
  std::vector<std::pair<u32, TransitionFunc>> transition;
  u32 flags = 0;
};

struct FSMBuilder {
  FSMBuilder(FSMNodeData* node_data) :
    node_data(node_data) {}

  FSMBuilder&
  Frame(r32 x, r32 y, r32 width, r32 height, u32 length)
  {
    node_data->frames.push_back({x, y, width, height, length});
    return *this;
  }

  FSMBuilder&
  Transition(u32 state, TransitionFunc func)
  {
    node_data->transition.push_back({state, func});
    return *this;
  }

  FSMBuilder&
  Flag(u32 flags)
  {
    node_data->flags |= flags;
    return *this;
  }

  FSMNodeData* node_data;
};

struct FSM {
  FSM(u32 num_states, u32 start_state) :
    node_data(num_states),
    current_state(start_state),
    // Start at -1 so first tick after creation sets the animation to the
    // first frame.
    current_frame(-1),
    ticks_in_frame(0) {}

  FSMBuilder
  Node(u32 state)
  {
    assert(state < node_data.size());
    return FSMBuilder(&node_data[state]);
  }

  void
  Update()
  {
    const FSMNodeData& nd = node_data[current_state];
    for (const auto& [state, func] : nd.transition) {
      if (func(entity)) {
        current_state = state;
        current_frame = 0;
        return;
      }
    }
    ++current_frame;
    if (current_frame >= nd.frames.size()) current_frame = 0;
  }

  const AnimFrame&
  Frame()
  {
    assert(current_state < node_data.size());
    const std::vector<AnimFrame>& af = node_data[current_state].frames;
    assert(current_frame < af.size());
    return af[current_frame];
  }

  void
  DebugPrint()
  {
    for (u32 i = 0; i < node_data.size(); ++i) {
      printf("ANIM %i [%s]\n", i, current_state == i ? "x" : ""); 
      for (u32 j = 0; j < node_data[i].frames.size(); ++j) {
        const auto& af = node_data[i].frames[j];
        printf("  %.2f, %.2f, %.2f, %.2f, %u [%s]\n",
               af.x, af.y, af.width, af.height, af.length,
               current_state == i && current_frame == j ? "x" : "");
      }
    }
  }

  std::vector<FSMNodeData> node_data;

  u32 current_state;
  u32 current_frame;
  // Number of ticks that have occurred in the current frame.
  u32 ticks_in_frame;

  // Entity this FSM belongs to.
  ecs::Entity entity;
};

int
main(int argc, char** argv)
{
  FSM adventurer_fsm(kAdventurerNumStates, kAdventurerIdle);

  adventurer_fsm
      .Node(kAdventurerIdle)
      .Frame(0.f, 37.f * 0.f, 50.f, 37.f, 25)
      .Frame(0.f, 37.f * 1.f, 50.f, 37.f, 25)
      .Frame(0.f, 37.f * 2.f, 50.f, 37.f, 25)
      .Frame(0.f, 37.f * 3.f, 50.f, 37.f, 25)
      .Transition(kAdventurerWalk,
                  [](ecs::Entity entity) {
                    printf("Transition to walk?\n");
                    return false;
                  })
      .Transition(kAdventurerJump,
                  [](ecs::Entity entity) {
                    printf("Transition to jump?\n");
                    return false;
                  });

  adventurer_fsm
      .Node(kAdventurerWalk)
      .Frame(50.f, 37.f * 1.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 2.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 3.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 4.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 5.f, 50.f, 37.f, 25)
      .Frame(50.f, 37.f * 6.f, 50.f, 37.f, 25)
      .Transition(kAdventurerIdle,
                  [](ecs::Entity entity) {
                    printf("Transition to walk?\n");
                    return false;
                  })
      .Transition(kAdventurerJump,
                  [](ecs::Entity entity) {
                    printf("Transition to jump?\n");
                    return false;
                  });

  adventurer_fsm.Update();
  adventurer_fsm.DebugPrint(); 

  adventurer_fsm.Update();
  adventurer_fsm.DebugPrint(); 

  adventurer_fsm.Update();
  adventurer_fsm.DebugPrint(); 

  adventurer_fsm.Update();
  adventurer_fsm.DebugPrint(); 

  adventurer_fsm.Update();
  adventurer_fsm.DebugPrint(); 

  std::vector<int> a;

  a.reserve(10);

  printf("%i\n", a[5]);

  a.push_back(3);

  printf("%i\n", a[0]);

  return 0;
}
