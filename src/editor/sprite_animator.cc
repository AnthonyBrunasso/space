#pragma once

DEFINE_EDITOR_CALLBACK(AssetBoxSelect, AssetSelection);

static const std::vector<std::string> kEditorKnownAssetExtensions = {
  "tga",
  "png",
  "anim",
};

bool EditorCanLoadAsset(const std::string& name) {
  for (const std::string& ext : kEditorKnownAssetExtensions) {
    if (filesystem::HasExtension(name.c_str(), ext.c_str())) return true;
  }
  return false;
}

class SpriteAnimatorControl : public EditorRenderTarget {
public:
  SpriteAnimatorControl();
  SpriteAnimatorControl(const SpriteAnimatorControl& anim) = delete;

  static void HandleAssetBoxSelect(const AssetSelection& selection);

  void OnRender() override;
  void OnImGui() override;

  bool IsMouseInside() const override;

  void AddFrame(const AnimFrame2d& frame, v2f dims, r32 duration = 1.f);
  void RemoveFrame(s32 idx);
  void Clear();
  void AddTimeToSequence(r32 time_sec);

  AnimSequence2d anim_sequence_;

  struct ModSpec {
    enum Type {
      kNone = 0,
      kSwap = 1,
      kRemove = 2,
    };
    Type type;
    s32 swap_idx1;
    s32 swap_idx2;
    s32 remove_idx;
  };

  struct Frame {
    r32 width() const { return editor_surface.width(); }
    r32 height() const { return editor_surface.height(); }
    EditorSurface editor_surface;
    void ImGui(AnimSequence2d::SequenceFrame& sframe, s32 id, ModSpec* mod_spec);
  };
  
  const std::vector<Frame>& anim_frames() const { return anim_frames_; }

  std::vector<Frame> anim_frames_;
  Rectf anim_frames_imgui_rect;
  char anim_filename_[128];
  bool is_running_ = true;
};

static SpriteAnimatorControl kSpriteAnimatorControl;

enum CursorMode {
  kCursorModeNone = 0,
  kClampToGridEdge = 1,
  kUseGridCell = 2,
};

class SpriteAnimator : public EditorRenderTarget {
public:
  void OnRender() override;
  void OnImGui() override;
  void OnFileSelected(const std::string& filename) override;

  const rgg::Texture* LoadTexture(const char* tname);

  void ChangeScale(r32 delta);

  rgg::TextureId texture_id_;
  CursorMode cursor_mode_ = kUseGridCell;
  bool clamp_cursor_to_nearest_ = true;
  bool clamp_cursor_to_rect_ = false;
  bool show_crosshair_ = true;
};

static SpriteAnimator kSpriteAnimator;

v2f EditorSpriteAnimatorTextureBottomLeft(const rgg::Texture& tex) {
  return v2f(-tex.width / 2.f, -tex.height / 2.f);
}

void EditorSpriteAnimatorRenderAsset() {
  const rgg::Texture* tex = rgg::GetTexture(kSpriteAnimator.texture_id_);
  if (!tex || !tex->IsValid()) return;
  Rectf dest = ScaleEditorRect(tex->Rect());
  rgg::RenderTexture(*tex, tex->Rect(), dest);
}

bool EditorSpriteAnimatorCursorInSelection() {
  if (kSpriteAnimatorControl.is_running_ && kSpriteAnimatorControl.IsMouseInside()) {
    return true;
  }
  return false;
}

const EditorCursor& EditorSpriteAnimatorCursor() {
  return kSpriteAnimator.cursor();
}

void SpriteAnimator::OnRender() {
  const rgg::Texture* texture = rgg::GetTexture(texture_id_);

  if (texture) {
    grid_.origin = EditorSpriteAnimatorTextureBottomLeft(*texture);
    //grid_.origin_offset = v2f(0.f, 0.f);
  }

  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];

  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));
  EditorSpriteAnimatorRenderAsset();

  RenderGrid(v4f(1.f, 1.f, 1.f, 0.2f), true);
  RenderAxis();

  if (cursor_.is_in_viewport && show_crosshair_ && !EditorSpriteAnimatorCursorInSelection()) {
    if (cursor_mode_ == kClampToGridEdge) {
      v2f scaled_clamp = cursor_.world_clamped * scale_;
      EditorRenderCrosshair(scaled_clamp, GetCameraRectScaled());
    } else if (cursor_mode_ == kUseGridCell) {
      RenderCursorAsRect();
    } else {
      EditorRenderCrosshair(cursor_.world_scaled, GetCameraRectScaled());
    }
  }

  if (kSpriteAnimatorSelection.action == 1) {
    EditorRenderCrosshair(kSpriteAnimatorSelection.start_world * scale_, GetCameraRectScaled(), rgg::kPurple);
  }
}

void SpriteAnimator::OnImGui() {
  UpdateImguiPanelRect();
  ImGuiImage();
}

void SpriteAnimator::OnFileSelected(const std::string& filename) {
  kSpriteAnimatorControl.Clear();
  const char* ext = filesystem::GetFilenameExtension(filename.c_str());
  if (strcmp(ext, "anim") == 0) {
    AnimSequence2d loaded_sequence;
    if (!AnimSequence2d::LoadFromProtoFile(filename.c_str(), &loaded_sequence)) {
      LOG(WARN, "Unable to load anim data %s", filename.c_str());
    } else {
      assert(!loaded_sequence.IsEmpty());
      r32 max_width = 0.f;
      r32 max_height = 0.f;
      for (const AnimSequence2d::SequenceFrame& sequence : loaded_sequence.sequence_frames_) {
        if (sequence.frame.src_rect().width > max_width) max_width = sequence.frame.src_rect().width;
        if (sequence.frame.src_rect().height > max_height) max_height = sequence.frame.src_rect().height;
      }
      v2f scaled_dims = v2f(max_width, max_height) * kSpriteAnimator.scale_;
      kSpriteAnimatorControl.Initialize(scaled_dims.x, scaled_dims.y);
      for (const AnimSequence2d::SequenceFrame& sequence_frame : loaded_sequence.sequence_frames_) {
        kSpriteAnimatorControl.AddFrame(sequence_frame.frame, scaled_dims, sequence_frame.duration_sec);
      }
      texture_id_ = kSpriteAnimatorControl.anim_sequence_.sequence_frames_[0].frame.texture_id_;
      kSpriteAnimatorControl.anim_sequence_.Start();
      std::string end_name = filesystem::Basename(filename.c_str());
      strncpy(kSpriteAnimatorControl.anim_filename_, end_name.c_str(), end_name.size());
    }
  } else {
    LoadTexture(filename.c_str());
  }
}

const rgg::Texture* SpriteAnimator::LoadTexture(const char* tname) {
  rgg::TextureInfo texture_info;
  texture_info.min_filter = GL_NEAREST_MIPMAP_NEAREST;
  texture_info.mag_filter = GL_NEAREST;
  texture_id_ = rgg::LoadTexture(tname, texture_info); 
  if (texture_id_ == 0) {
    LOG(WARN, "Unable to load asset %s", tname);
    return nullptr;
  }
  return rgg::GetTexture(texture_id_);
}

void SpriteAnimator::ChangeScale(r32 delta) {
  if (scale_ + delta > 0.f && scale_ + delta <= 15.f)
    scale_ += delta;
}

void SpriteAnimatorControl::HandleAssetBoxSelect(const AssetSelection& selection) {
  AnimFrame2d frame;
  frame.texture_id_ = kSpriteAnimator.texture_id_;
  frame.src_rect_ = selection.tex_rect;
  kSpriteAnimatorControl.AddFrame(frame, selection.world_rect_scaled.Dims(), 1.f);
  kSpriteAnimatorControl.anim_sequence_.Start();
  if (!kSpriteAnimatorControl.IsRenderTargetValid()) {
    kSpriteAnimatorControl.Initialize((s32)selection.world_rect_scaled.width, (s32)selection.world_rect_scaled.height);
  } else {
    // Find the max width and height of the frames.
    r32 max_width = 0.f;
    r32 max_height = 0.f;
    for (const Frame& frame : kSpriteAnimatorControl.anim_frames()) {
      if (frame.width() > max_width) max_width = frame.width();
      if (frame.height() > max_height) max_height = frame.height();
    }

    if (max_width != kSpriteAnimatorControl.GetRenderTargetWidth() ||
        max_height != kSpriteAnimatorControl.GetRenderTargetHeight()) {
      kSpriteAnimatorControl.ReleaseSurface();
      kSpriteAnimatorControl.Initialize(max_width, max_height);
    }
  }
}

SpriteAnimatorControl::SpriteAnimatorControl() {
  SubscribeAssetBoxSelect(&HandleAssetBoxSelect);
}

void SpriteAnimatorControl::OnRender() {
  anim_sequence_.Update();

  if (!anim_sequence_.IsEmpty()) {
    grid_.origin = GetRenderTargetDims() / -2.f;
    RenderGrid(v4f(1.f, 1.f, 1.f, 0.2f));
    //const SpriteAnimatorFrame* cf = &kSpriteAnimator.frames_[anim_sequence_.frame_index_];
    const AnimFrame2d& aframe = anim_sequence_.CurrentFrame();
    const rgg::Texture* texture_render_from = rgg::GetTexture(aframe.texture_id_);
    Rectf src = aframe.src_rect();
    Rectf dest = Rectf(GetRenderTargetBottomLeft(),
                         v2f(src.width * kSpriteAnimator.scale_,
                             src.height * kSpriteAnimator.scale_));
    rgg::RenderTexture(*texture_render_from, src, dest);
  }
}

void SpriteAnimatorControl::Clear() {
  anim_sequence_.Clear();
  ReleaseSurface();
  for (Frame& frame : anim_frames_) {
    DestroyEditorSurface(&frame.editor_surface);
  }
  anim_frames_.clear();
  memset(anim_filename_, 0, 128);
}

void SpriteAnimatorControl::AddTimeToSequence(r32 time_sec) {
  for (AnimSequence2d::SequenceFrame& sequence_frame : anim_sequence_.sequence_frames_) {
    sequence_frame.duration_sec += time_sec;
  }
}

void SpriteAnimatorControl::OnImGui() {
  ImGui::Begin("Animator", &kSpriteAnimatorControl.is_running_);
  ImGuiImage();
  // Walk all the frames at a certain cadence and play them.
  if (anim_sequence_.sequence_frames_.size() > 0) {
    ImGui::Text("clock: %.2f (prev: %.2f / next: %.2f)",
                platform::ClockDeltaSec(anim_sequence_.clock_),
                anim_sequence_.last_frame_time_sec_,
                anim_sequence_.next_frame_time_sec_);

    if (ImGui::Button("-1s")) AddTimeToSequence(-1.f);
    ImGui::SameLine();
    if (ImGui::Button("-.1s")) AddTimeToSequence(-.1f);
    ImGui::SameLine();
    if (ImGui::Button("-.01s")) AddTimeToSequence(-.01f);

    if (ImGui::Button("+1s")) AddTimeToSequence(1.f);
    ImGui::SameLine();
    if (ImGui::Button("+.1s")) AddTimeToSequence(.1f);
    ImGui::SameLine();
    if (ImGui::Button(".01s")) AddTimeToSequence(.01f);

    if (ImGui::Button("reset clock")) {
      anim_sequence_.Start();
    }
    ImGui::SameLine();
    if (ImGui::Button("reverse frames")) {
      std::reverse(anim_frames_.begin(), anim_frames_.end());
      anim_sequence_.ReverseFrames();
    }
  }
  ImGui::Separator();
  static char kFullPath[256];
  ImGui::InputText("file", anim_filename_, 128); 
  snprintf(kFullPath, 256, "gamedata/%s.anim", anim_filename_);
  ImGui::Text("%s", kFullPath);
  if (ImGui::Button("save")) {
    proto::Animation2d proto = anim_sequence_.ToProto();
    LOG(INFO, "Saving animation as proto %s to file %s",
        proto.DebugString().c_str(), kFullPath);
    std::fstream fo(kFullPath, std::ios::binary | std::ios::out);
    proto.SerializeToOstream(&fo);
    fo.close();
  }
  ImGui::SameLine();
  if (ImGui::Button("load")) {
    std::string lfile(kFullPath);
    kSpriteAnimator.OnFileSelected(lfile);
  }
  ImGui::SameLine();
  if (ImGui::Button("clear")) {
    Clear();
  }
  
  UpdateImguiPanelRect();
  ImGui::End();

  if (!anim_sequence_.IsEmpty()) {
    assert(anim_sequence_.FrameCount() == anim_frames_.size());
    int i = 0;
    v2f wsize = window::GetWindowSize();
    r32 frame_view_width = wsize.x - kExplorerWidth;
    r32 frame_view_y = wsize.y - kFrameRendererHeight;
    ImVec2 imsize = ImVec2((float)frame_view_width, (float)kFrameRendererHeight);
    ImGui::SetNextWindowSize(imsize);
    ImGui::SetNextWindowPos(ImVec2((float)kRenderViewStart, (float)frame_view_y));
    ImGui::Begin("Frames", nullptr);
    if (ImGui::BeginTable(
          "FramesTable", (int)anim_sequence_.sequence_frames_.size(), ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX )) {
      ModSpec mod_spec;
      for (AnimSequence2d::SequenceFrame& sequence_frame : anim_sequence_.sequence_frames_) {
        ImGui::TableNextColumn();
        Frame* render_frame = &anim_frames_[i];
        render_frame->ImGui(sequence_frame, i, &mod_spec);
        if (mod_spec.type == ModSpec::kSwap) {
          if (anim_sequence_.sequence_frames_.size() > 1) {
            mod_spec.swap_idx1 %= anim_sequence_.sequence_frames_.size();
            mod_spec.swap_idx2 %= anim_sequence_.sequence_frames_.size();
            AnimSequence2d::SequenceFrame t = anim_sequence_.sequence_frames_[mod_spec.swap_idx1];
            anim_sequence_.sequence_frames_[mod_spec.swap_idx1] =
                anim_sequence_.sequence_frames_[mod_spec.swap_idx2];
            anim_sequence_.sequence_frames_[mod_spec.swap_idx2] = t;
            break;
          }
        } else if (mod_spec.type == ModSpec::kRemove) {
          RemoveFrame(mod_spec.remove_idx);
          break;
        }
        ++i;
      }
      ImGui::EndTable();
    }
    GetImGuiPanelRect(&anim_frames_imgui_rect);
    ImGui::End();
  }
}

void SpriteAnimatorControl::Frame::ImGui(AnimSequence2d::SequenceFrame& sframe, s32 id, ModSpec* mod_spec) {
  const rgg::Texture* texture = rgg::GetTexture(sframe.frame.texture_id_);
  // At this point if the texture isn't loaded we have a deeper problem. Load textures before animating.
  assert(texture != nullptr);

  RenderSurfaceToImGuiImage(editor_surface, texture, sframe.frame.src_rect(), sframe.is_active);

  static char kTempStr[128];
  snprintf(kTempStr, 128, "-##x%i", id);
  if (ImGui::Button(kTempStr)) sframe.frame.src_rect_.x += 1.f;
  ImGui::SameLine();
  snprintf(kTempStr, 128, "+##x%i", id);
  if (ImGui::Button(kTempStr)) sframe.frame.src_rect_.x -= 1.f;
  ImGui::SameLine();
  ImGui::Text("x");
  ImGui::SameLine();
  snprintf(kTempStr, 128, "-##y%i", id);
  if (ImGui::Button(kTempStr)) sframe.frame.src_rect_.y += 1.f;
  ImGui::SameLine();
  snprintf(kTempStr, 128, "+##y%i", id);
  if (ImGui::Button(kTempStr)) sframe.frame.src_rect_.y -= 1.f;
  ImGui::SameLine();
  ImGui::Text("y");

  //ImGui::Text("t: %.2f", sframe.duration_sec);
  snprintf(kTempStr, 128, "time##%i", id);
  ImGui::SliderFloat(kTempStr, &sframe.duration_sec, 0.1f, 2.f, "%.01f", ImGuiSliderFlags_None);

  mod_spec->type = ModSpec::kNone;

  snprintf(kTempStr, 128, "x##%i", id);
  if (ImGui::Button(kTempStr)) {
    mod_spec->type = ModSpec::kRemove;
    mod_spec->remove_idx = id;
  }

  ImGui::SameLine();

  snprintf(kTempStr, 128, "<##%i", id);
  if (ImGui::Button(kTempStr)) {
    mod_spec->type = ModSpec::kSwap;
    mod_spec->swap_idx1 = id - 1;
    mod_spec->swap_idx2 = id;
  }
  ImGui::SameLine();
  snprintf(kTempStr, 128, ">##%i", id);
  if (ImGui::Button(kTempStr)) {
    mod_spec->type = ModSpec::kSwap;
    mod_spec->swap_idx1 = id;
    mod_spec->swap_idx2 = id + 1;
  }

  // The min we can possibly leave an open frame.
  if (sframe.duration_sec < 0.01f) sframe.duration_sec = 0.01f;
}

bool SpriteAnimatorControl::IsMouseInside() const {
  if (EditorRenderTarget::IsMouseInside()) return true;
  for (const Frame& frame : anim_frames_) {
    if (math::PointInRect(EditorSpriteAnimatorCursor().global_screen, anim_frames_imgui_rect)) return true;
  }
  return false;
}

void SpriteAnimatorControl::AddFrame(const AnimFrame2d& frame, v2f dims, r32 duration) {
  anim_sequence_.AddFrame(frame, duration);
  Frame asset_frame;
  asset_frame.editor_surface = CreateEditorSurface(dims.x, dims.y);
  assert(asset_frame.editor_surface.IsValid());
  anim_frames_.push_back(asset_frame);
}

void SpriteAnimatorControl::RemoveFrame(s32 idx) {
  assert(idx >= 0 && idx < anim_frames_.size() && idx < anim_sequence_.sequence_frames_.size());
  Frame* asset_frame = &anim_frames_[idx];
  DestroyEditorSurface(&asset_frame->editor_surface);
  anim_frames_.erase(anim_frames_.begin() + idx);
  anim_sequence_.sequence_frames_.erase(anim_sequence_.sequence_frames_.begin() + idx);
  anim_sequence_.Start();
}

v2f EditorSpriteAnimatorCursorInTexture(const rgg::Texture& texture) {
  v2f world_to_texture;
  if (kSpriteAnimator.cursor_mode_ == kClampToGridEdge) {
    world_to_texture = EditorSpriteAnimatorCursor().world_clamped + (texture.Rect().Dims() / 2.0);
  } else {
    world_to_texture = EditorSpriteAnimatorCursor().world + (texture.Rect().Dims() / 2.0);
  }
  return math::Roundf(world_to_texture);
}

v2f EditorSpriteAnimatorCursorWorld() {
  if (kSpriteAnimator.cursor_mode_ == kClampToGridEdge) {
    return EditorSpriteAnimatorCursor().world_clamped;
  }
  return EditorSpriteAnimatorCursor().world;
}

rgg::Camera* EditorSpriteAnimatorCamera() {
  return kSpriteAnimator.camera();
}

void ProcessSelectionForClampedCursor(const rgg::Texture* texture) {
  if (kSpriteAnimatorSelection.action == 2) {
    kSpriteAnimatorSelection = {};
    return;
  }
  if (kSpriteAnimatorSelection.action == 0) {
    kSpriteAnimatorSelection.start_texcoord =
        EditorSpriteAnimatorCursorInTexture(*texture);
    kSpriteAnimatorSelection.start_world = EditorSpriteAnimatorCursorWorld();
    kSpriteAnimatorSelection.action = 1;
  }
  else if (kSpriteAnimatorSelection.action == 1) {
    kSpriteAnimatorSelection.end_texcoord =
        EditorSpriteAnimatorCursorInTexture(*texture);
    kSpriteAnimatorSelection.end_world = EditorSpriteAnimatorCursorWorld();
    kSpriteAnimatorSelection.action = 2;
    AssetSelection selection;
    selection.tex_rect = math::MakeRect(
        kSpriteAnimatorSelection.start_texcoord, kSpriteAnimatorSelection.end_texcoord);
    selection.world_rect = math::MakeRect(
        kSpriteAnimatorSelection.start_world, kSpriteAnimatorSelection.end_world);
    selection.world_rect_scaled = kSpriteAnimatorSelection.WorldRectScaled();
    DispatchAssetBoxSelect(selection);
  }
}

void ProcessSelectionForRect(const rgg::Texture* texture) {
  AssetSelection selection;
  selection.tex_rect = EditorSpriteAnimatorCursor().world_grid_cell;
  selection.tex_rect.x += texture->width / 2.f;
  selection.tex_rect.y += texture->height / 2.f;
  selection.world_rect = EditorSpriteAnimatorCursor().world_grid_cell;
  selection.world_rect_scaled = ScaleRect(selection.world_rect);
  DispatchAssetBoxSelect(selection);
}

void EditorSpriteAnimatorProcessEvent(const PlatformEvent& event) {
  kSpriteAnimator.UpdateCursor();
  kSpriteAnimatorControl.UpdateCursor();

  switch(event.type) {
    case MOUSE_DOWN: {
      switch (event.key) {
        case BUTTON_LEFT: {
          if (EditorSpriteAnimatorCursorInSelection()) {
            break;
          }
          // Not viewing an asset.
          const rgg::Texture* texture = rgg::GetTexture(kSpriteAnimator.texture_id_);
          if (!texture || !texture->IsValid()) break;
          // Cursor isn't in the viewer.
          if (!EditorSpriteAnimatorCursor().is_in_viewport) break;
          if (kSpriteAnimator.cursor_mode_ == kClampToGridEdge) ProcessSelectionForClampedCursor(texture);
          else if (kSpriteAnimator.cursor_mode_ == kUseGridCell) ProcessSelectionForRect(texture);
        } break;
      } break;
    } break;
    case KEY_DOWN: {
      switch (event.key) {
        case KEY_NUMPAD_UP:
        case KEY_ARROW_UP: {
          rgg::Camera* camera = EditorSpriteAnimatorCamera();
          if (camera) {
            camera->position += v2f(0.f, ScaleR32(16.f));
          }
        } break;
        case KEY_NUMPAD_RIGHT:
        case KEY_ARROW_RIGHT: {
          rgg::Camera* camera = EditorSpriteAnimatorCamera();
          if (camera) {
            camera->position += v2f(ScaleR32(16.f), 0.f);
          }
        } break;
        case KEY_NUMPAD_DOWN:
        case KEY_ARROW_DOWN: {
          rgg::Camera* camera = EditorSpriteAnimatorCamera();
          if (camera) {
            camera->position += v2f(0.f, ScaleR32(-16.f));
          }
        } break;
        case KEY_NUMPAD_LEFT:
        case KEY_ARROW_LEFT: {
          rgg::Camera* camera = EditorSpriteAnimatorCamera();
          if (camera) {
            camera->position += v2f(ScaleR32(-16.f), 0.f);
          }
        } break;
      }
    } break;
    case MOUSE_WHEEL: {
      if (kSpriteAnimator.IsMouseInsideEditorSurface() && !EditorSpriteAnimatorCursorInSelection()) {
        if (event.wheel_delta > 0) {
          kSpriteAnimator.ChangeScale(1.f);
        } else if (event.wheel_delta < 0) {
          kSpriteAnimator.ChangeScale(-1.f);
        }
      }
    } break;
    case MOUSE_UP:
    case NOT_IMPLEMENTED:
    default: break;
  }
}

void EditorSpriteAnimatorInitialize() {
  static bool do_once = true;
  if (!do_once) {
    return;
  }
  kSpriteAnimator.Initialize((s32)kEditor.render_viewport.width, (s32)kEditor.render_viewport.height);
  do_once = false;
}

void EditorSpriteAnimatorMain() {
  EditorSetCurrent(&kSpriteAnimator);
  EditorSpriteAnimatorInitialize();
  kSpriteAnimator.Render();
  if (kSpriteAnimatorControl.is_running_) {
    kSpriteAnimatorControl.Render();
  }

  if (kSpriteAnimatorSelection.action == 2) {
    Rectf selection_rect_scaled = kSpriteAnimatorSelection.WorldRectScaled();
    kSpriteAnimatorSelection.action = 0;
  }
}

void EditorSpriteAnimatorDebug() {
  const rgg::Texture* texture = rgg::GetTexture(kSpriteAnimator.texture_id_);
  if (texture && texture->IsValid()) {
    ImGui::Text("File (%s)", filesystem::Filename(texture->file).c_str());
    ImGui::Text("  texture id    %u", kSpriteAnimator.texture_id_);
    ImGui::Text("  dims          %.0f %.0f", texture->width, texture->height);
    v2f cursor_in_texture = EditorSpriteAnimatorCursorInTexture(*texture);
    ImGui::Text("  texcoord      %.2f %.2f", cursor_in_texture.x, cursor_in_texture.y);
    ImGui::NewLine();
  }
  static const char* kCursorModesStr[] = {
    "None",
    "Clamp Grid Edge",
    "Use Grid Cell",
  };
  ImGui::Combo("cursor", (s32*)&kSpriteAnimator.cursor_mode_, kCursorModesStr, 3);
  ImGui::SliderFloat("scale", &kSpriteAnimator.scale_, 1.f, 15.f, "%.0f", ImGuiSliderFlags_None);
  ImGui::Checkbox("render crosshair", &kSpriteAnimator.show_crosshair_);
  ImGui::NewLine();
  EditorDebugMenuGrid(kSpriteAnimator.grid());
}

r32 EditorSpriteAnimatorScale() {
  return kSpriteAnimator.scale_;
}
