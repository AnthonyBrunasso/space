#pragma once

#include "animation.pb.h"

// A specific frame in a sequence.
class AnimFrame2d {
public:
  Rectf src_rect() const {
    return src_rect_;
  }
  // Src texture this frame was taken frame.
  rgg::TextureId texture_id_;
  // The texture coordinates to grab from texture_id.
  Rectf src_rect_;
};

// A list of frames and the relevant logic to cycle through them.
class AnimSequence2d {
public:
  struct SequenceFrame {
    AnimFrame2d frame;
    r32 duration_sec = 1.f;
    b8 is_active = false;
  };

  void Start();
  void Update();
  const AnimFrame2d& CurrentFrame();

  void AddFrame(const AnimFrame2d& frame, r32 duration_sec);

  // Clear all internal data.
  void Clear();
  s32 FrameCount() const { return (s32)sequence_frames_.size(); }
  bool IsEmpty() const { return sequence_frames_.empty(); }

  proto::Animation2d ToProto() const;

  std::vector<SequenceFrame> sequence_frames_;
  r32 last_frame_time_sec_;
  r32 next_frame_time_sec_;
  platform::Clock clock_;
  s32 frame_index_ = 0;
};

void AnimSequence2d::Start() {
  // Need at least two frames to have a sequence.
  assert(sequence_frames_.size() >= 1);
  platform::ClockStart(&clock_);
  frame_index_ = 0;
  last_frame_time_sec_ = platform::ClockDeltaSec(clock_);
  next_frame_time_sec_ = last_frame_time_sec_ + sequence_frames_[frame_index_].duration_sec;
  sequence_frames_[frame_index_].is_active = true;
}

void AnimSequence2d::Update() {
  // A frame was probably removed causing the current animation to be invalid, so just restart it.
  if (frame_index_ >= sequence_frames_.size()) {
    Start();
    return;
  }

  r32 now = platform::ClockDeltaSec(clock_);
  if (now >= next_frame_time_sec_) {
    sequence_frames_[frame_index_].is_active = false;
    last_frame_time_sec_ = next_frame_time_sec_;
    s32 pre_index = frame_index_;
    frame_index_ += 1;
    frame_index_ = (frame_index_ % sequence_frames_.size());
    next_frame_time_sec_ += sequence_frames_[frame_index_].duration_sec;
    sequence_frames_[frame_index_].is_active = true;
  }

  platform::ClockEnd(&clock_);
}

void AnimSequence2d::AddFrame(const AnimFrame2d& frame, r32 duration_sec) {
  SequenceFrame fr;
  fr.frame = frame;
  fr.duration_sec = duration_sec;
  sequence_frames_.push_back(fr);
}

const AnimFrame2d& AnimSequence2d::CurrentFrame() {
  assert(frame_index_ < sequence_frames_.size());
  return sequence_frames_[frame_index_].frame;
}

void AnimSequence2d::Clear() {
  sequence_frames_.clear();
  clock_ = {};
  last_frame_time_sec_ = 0.f;
  next_frame_time_sec_ = 0.f;
  frame_index_ = 0;
}

proto::Animation2d AnimSequence2d::ToProto() const {
  proto::Animation2d proto;
  for (const SequenceFrame& sframe : sequence_frames_) {
    const rgg::Texture* atex = rgg::GetTexture(sframe.frame.texture_id_);
    assert(atex != nullptr);
    proto::AnimationFrame2d* aframe = proto.add_frames();
    aframe->set_asset_name(atex->file);
    aframe->set_texture_x(sframe.frame.src_rect_.x);
    aframe->set_texture_y(sframe.frame.src_rect_.y);
    aframe->set_texture_width(sframe.frame.src_rect_.width);
    aframe->set_texture_height(sframe.frame.src_rect_.height);
    aframe->set_duration_sec(sframe.duration_sec);
  }
  return proto;
}
