#pragma once

#include <vector>
#include <functional>

namespace animation
{

struct AnimFrame {
  r32 x;
  r32 y;
  r32 width;
  r32 height;
  u32 length;

  Rectf
  rect() const
  {
    return Rectf(x, y, width, height);
  }
};

typedef std::function<b8(u32)> TransitionFunc;

enum FSMNodeFlags {
  // Animation will freeze on the final frame instead of looping.
  kFSMNodeStopOnFinalFrame = 0,
  // Animation cannot be transitioned out of until it is finished.
  kFSMNodePlayUntilComplete = 1,
  // If set gampleay should not move the character while this node runs.
  kFSMNodeCantMove = 2,
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
  Flag(u32 flag)
  {
    SBIT(node_data->flags, flag);
    return *this;
  }

  FSMNodeData* node_data;
};

struct FSM {
  FSM() :
    node_data(),
    current_state(0),
    current_frame(-1),
    ticks_in_frame(0) {}

  FSM(u32 num_states, u32 start_state) :
    node_data(num_states),
    current_state(start_state),
    // Start at -1 so first tick after creation sets the animation to the
    // first frame.
    current_frame(-1),
    ticks_in_frame(0) {}

  void
  Initialize(u32 num_states, u32 start_state)
  {
    node_data.resize(num_states);
    current_state = start_state;
    current_frame = -1;
    ticks_in_frame = 0;
  }

  FSMBuilder
  Node(u32 state)
  {
    assert(state < node_data.size());
    return FSMBuilder(&node_data[state]);
  }

  void
  Update(u32 entity_id)
  {
    assert(current_state < node_data.size());
    const FSMNodeData& nd = node_data[current_state];

    if (!FLAGGED(nd.flags, kFSMNodePlayUntilComplete) ||
        current_frame == nd.frames.size() - 1) {
      for (const auto& [state, func] : nd.transition) {
        if (func(entity_id)) {
          current_state = state;
          current_frame = 0;
          return;
        }
      }
    }

    if (FLAGGED(nd.flags, kFSMNodeStopOnFinalFrame) &&
        current_frame == nd.frames.size() - 1) {
      return;
    }

    ++ticks_in_frame;
    if (current_frame == u32(-1) ||
        ticks_in_frame > nd.frames[current_frame].length) {
      ++current_frame;
      if (current_frame >= nd.frames.size()) current_frame = 0;
      ticks_in_frame = 0;
    }
  }

  const AnimFrame&
  Frame()
  {
    assert(current_state < node_data.size());
    const std::vector<AnimFrame>& af = node_data[current_state].frames;
    assert(current_frame < af.size());
    return af[current_frame];
  }

  u32
  Flags() const
  {
    assert(current_state < node_data.size());
    return node_data[current_state].flags;
  }

  bool
  CanInterrupt() const
  {
    assert(current_state < node_data.size());
    const FSMNodeData& nd = node_data[current_state];
    return !FLAGGED(nd.flags, kFSMNodePlayUntilComplete);
  }

  void
  DebugPrint()
  {
    for (u32 i = 0; i < node_data.size(); ++i) {
      LOG(INFO, "ANIM %i [%s]", i, current_state == i ? "x" : ""); 
      for (u32 j = 0; j < node_data[i].frames.size(); ++j) {
        const auto& af = node_data[i].frames[j];
        LOG(INFO, "  %.2f, %.2f, %.2f, %.2f, %u [%s]",
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
  u32 entity_id = 0;
};

}  // namespace animation
