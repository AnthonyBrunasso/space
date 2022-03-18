#pragma once

static const std::vector<std::string> kEditorKnownAssetExtensions = {
  "tga",
  "png",
};

bool EditorCanLoadAsset(const std::string& name) {
  for (const std::string& ext : kEditorKnownAssetExtensions) {
    if (filesystem::HasExtension(name.c_str(), ext.c_str())) return true;
  }
  return false;
}

class AssetFrame {
public:
  Rectf src_rect() const {
    Rectf src = src_rect_;
    src.x += src_rect_offset_.x;
    src.y += src_rect_offset_.y;
    return src;
  }
  // Src texture this frame was taken frame.
  rgg::TextureId texture_id_;
  // The texture coordinates to grab from texture_id.
  Rectf src_rect_;
  // How big to render the resulting frame.
  Rectf dest_rect_;
  // Allows for offsetting texture coordinates
  v2f src_rect_offset_;
};

class AssetViewerFrame : public EditorRenderTarget {
public:
  static AssetViewerFrame Create(rgg::TextureId texture_id, const Rectf& selection_rect_scaled);
  void OnRender() override;
  void OnImGui() override;
  void SetIsAnimating(bool val) { is_animating_ = val; }


  // Texture and src / dest rect.
  AssetFrame frame_;
  // Unique id of the asset viewer frame.
  u32 id_;
  // Whether this thing should render or not.
  bool is_running_;
  // If true will set a red outline in the frame to indicate this is the currently animating frame.
  bool is_animating_;
};

class AssetViewerAnimator : public EditorRenderTarget {
public:
  void ResetClock();
  void UpdateFrameTimes();

  void OnInitialize() override;
  void OnRender() override;
  void OnImGui() override;

  platform::Clock clock_;
  // Frequency to switch frames in seconds.
  r32 frequency_;
  r32 last_frame_time_sec_;
  r32 next_frame_time_sec_;
  s32 frame_index_;
  bool is_running_;
};

static AssetViewerAnimator kAssetViewerAnimator;

class AssetViewer : public EditorRenderTarget {
public:
  void OnRender() override;
  void OnImGui() override;

  rgg::TextureId texture_id_;
  r32 scale_ = 1.0f;
  std::string chosen_asset_path_;
  std::vector<AssetViewerFrame> frames_;
  bool clamp_cursor_to_nearest_ = true;
  bool show_crosshair_ = true;
  bool animate_frames_ = false;
};

static AssetViewer kAssetViewer;

v2f EditorAssetViewerTextureBottomLeft(const rgg::Texture& tex) {
  return v2f(-tex.width / 2.f, -tex.height / 2.f);
}

void EditorAssetViewerRenderAsset() {
  const rgg::Texture* tex = rgg::GetTexture(kAssetViewer.texture_id_);
  if (!tex || !tex->IsValid()) return;
  Rectf dest = ScaleEditorRect(tex->Rect());
  rgg::RenderTexture(*tex, tex->Rect(), dest);
}

bool EditorAssetViewerCursorInSelection() {
  if (kAssetViewerAnimator.is_running_ && kAssetViewerAnimator.IsMouseInside()) {
    return true;
  }
  for (const AssetViewerFrame& frame : kAssetViewer.frames_) {
    if (frame.IsMouseInside()) {
      return true;
    }
  }
  return false;
}

void AssetViewer::OnRender() {
  const rgg::Texture* texture = nullptr;
  if (!chosen_asset_path_.empty()) {
    rgg::TextureInfo texture_info;
    texture_info.min_filter = GL_NEAREST_MIPMAP_NEAREST;
    texture_info.mag_filter = GL_NEAREST;
    texture_id_ = rgg::LoadTexture(chosen_asset_path_.c_str(), texture_info); 
    if (texture_id_ == 0) {
      LOG(WARN, "Unable to load asset %s", chosen_asset_path_.c_str());
    }
    else {
      texture = rgg::GetTexture(texture_id_);
      kGrid.origin = EditorAssetViewerTextureBottomLeft(*texture);
      kGrid.origin_offset = v2f(0.f, 0.f);
      chosen_asset_path_.clear();
    }
  }

  texture = rgg::GetTexture(texture_id_);
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(ScaleEditorViewport(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));
  EditorAssetViewerRenderAsset();

  v2f origin_scaled = ScaleVec2(v2f(0.f, 0.f));
  if (texture && texture->IsValid()) {
    origin_scaled = ScaleVec2(kGrid.GetOrigin());
  }

  EditorRenderGrid(origin_scaled, kGrid, v4f(1.f, 1.f, 1.f, 0.2f));
  EditorRenderAxis(origin_scaled, ScaleEditorViewport());

  // Useful for debugging cursor stuff
  //rgg::RenderLine(kCursor.world, v2f(0.f, 0.f), rgg::kWhite);

  if (kCursor.is_in_viewport && show_crosshair_ && !EditorAssetViewerCursorInSelection()) {
    if (clamp_cursor_to_nearest_) {
      v2f scaled_clamp = kCursor.world_clamped * scale_;
      EditorRenderCrosshair(scaled_clamp, ScaleEditorViewport());
    }
    else {
      EditorRenderCrosshair(kCursor.world_clamped, ScaleEditorViewport());
    }
  }

  if (kAssetViewerSelection.action == 1) {
    EditorRenderCrosshair(kAssetViewerSelection.start_world * scale_, ScaleEditorViewport(), rgg::kPurple);
  }

  for (const AssetViewerFrame& frame : frames_) {
    if (frame.frame_.texture_id_ == texture_id_) {
      rgg::RenderLineRectangle(frame.frame_.dest_rect_, rgg::kPurple);
    }
  }
}

void AssetViewer::OnImGui() {
  ImGuiImage();
}

void AssetViewerFrame::OnRender() {
  if (!IsRenderTargetValid()) {
    is_running_ = false;
    return;
  }
  const rgg::Texture* texture_render_from = rgg::GetTexture(frame_.texture_id_);
  assert(texture_render_from);
  Rectf target_rect(
      -frame_.dest_rect_.width / 2.f, -frame_.dest_rect_.height / 2.f,
      frame_.dest_rect_.width, frame_.dest_rect_.height);
  rgg::RenderTexture(*texture_render_from, frame_.src_rect(), target_rect);
  if (is_animating_) {
    Rectf visible_rect = target_rect;
    // First pixel appears clipped out, so move the frame a bit to see the red outline.
    visible_rect.x += 1.f;
    visible_rect.width -= 1.f;
    visible_rect.height -= 1.f;
    rgg::RenderLineRectangle(visible_rect, rgg::kRed);
  }
}

void AssetViewerFrame::OnImGui() {
  const rgg::Texture* texture_render_from = rgg::GetTexture(frame_.texture_id_);
  assert(texture_render_from);
  bool open = true;
  char panel_name[128];
  snprintf(panel_name, 128, "%s/%i", filesystem::Filename(texture_render_from->file).c_str(), id_);
  ImGui::Begin(panel_name, &open);
  UpdateImguiPanelRect();
  ImGuiImage();
  ImGui::SliderFloat("offsetx", &frame_.src_rect_offset_.x, -16.f, 16.f, "%.0f");
  ImGui::SliderFloat("offsety", &frame_.src_rect_offset_.y, -16.f, 16.f, "%.0f");
  ImGui::End();
  if (open == false) {
    ReleaseSurface();
    is_running_ = false;
  }
}

AssetViewerFrame AssetViewerFrame::Create(rgg::TextureId texture_id, const Rectf& selection_rect_scaled) {
  AssetViewerFrame frame;
  frame.Initialize(selection_rect_scaled.width, selection_rect_scaled.height);
  frame.is_animating_ = false;
  frame.is_running_ = true;
  frame.frame_.dest_rect_ = selection_rect_scaled;
  frame.frame_.texture_id_ = texture_id;
  frame.frame_.src_rect_ = kAssetViewerSelection.TexRect();
  static u32 kUniqueId = 1;
  frame.id_ = kUniqueId++;
  return frame;
}

void AssetViewerAnimator::ResetClock() {
  platform::ClockStart(&clock_);
}

void AssetViewerAnimator::OnInitialize() {
  ResetClock();
  is_running_ = true;
  frame_index_ = 0;
  if (!kAssetViewer.frames_.empty()) {
    kAssetViewer.frames_[frame_index_].SetIsAnimating(true);
  }
  frequency_ = 1.f;
  last_frame_time_sec_ = platform::ClockDeltaSec(clock_);
  next_frame_time_sec_ = last_frame_time_sec_ + frequency_;
}

void AssetViewerAnimator::UpdateFrameTimes() {
  if (kAssetViewer.frames_.empty())
    return;

  r32 now = platform::ClockDeltaSec(clock_);
  if (now >= next_frame_time_sec_) {
    last_frame_time_sec_ = next_frame_time_sec_;
    next_frame_time_sec_ += frequency_;
    s32 pre_index = frame_index_;
    frame_index_ += 1;
    frame_index_ = (frame_index_ % kAssetViewer.frames_.size());
    kAssetViewer.frames_[pre_index].SetIsAnimating(false);
    kAssetViewer.frames_[frame_index_].SetIsAnimating(true);
  }
}

void AssetViewerAnimator::OnRender() {
  UpdateFrameTimes();

  const AssetViewerFrame* cf = nullptr;
  if (kAssetViewer.frames_.size() > 0) {
    cf = &kAssetViewer.frames_[frame_index_];
    const rgg::Texture* texture_render_from = rgg::GetTexture(cf->frame_.texture_id_);
    rgg::RenderTexture(*texture_render_from,
                       cf->frame_.src_rect(),
                       Rectf(-cf->frame_.dest_rect_.width / 2.f, -cf->frame_.dest_rect_.height / 2.f,
                             cf->frame_.dest_rect_.width, cf->frame_.dest_rect_.height));
  }
}

void AssetViewerAnimator::OnImGui() {
  ImGui::Begin("Animator", &kAssetViewerAnimator.is_running_);
  ImGuiImage();
  UpdateImguiPanelRect();
  // Walk all the frames at a certain cadence and play them.
  ImGui::SliderFloat("frequency", &frequency_, 0.f, 2.f, "%.01f", ImGuiSliderFlags_None);
  if (ImGui::Button("reset clock")) {
    ResetClock();
  }
  ImGui::Text("Clock: %.2f", platform::ClockDeltaSec(clock_));
  ImGui::Text("%i %.2f / %.2f", frame_index_, last_frame_time_sec_, next_frame_time_sec_);
  ImGui::End();
  platform::ClockEnd(&clock_);
}

v2f EditorAssetViewerCursorInTexture(const rgg::Texture& texture) {
  v2f world_to_texture;
  if (kAssetViewer.clamp_cursor_to_nearest_) {
    world_to_texture = kCursor.world_clamped + (texture.Rect().Dims() / 2.0);
  }
  else {
    world_to_texture = kCursor.world + (texture.Rect().Dims() / 2.0);
  }
  return math::Roundf(world_to_texture);
}

v2f EditorAssetViewerCursorWorld() {
  if (kAssetViewer.clamp_cursor_to_nearest_) {
    return kCursor.world_clamped;
  }
  return kCursor.world;
}

rgg::Camera* EditorAssetViewerCamera() {
  return kAssetViewer.camera();
}

void EditorAssetViewerProcessEvent(const PlatformEvent& event) {
  switch(event.type) {
    case MOUSE_DOWN: {
      switch (event.key) {
        case BUTTON_LEFT: {
          if (EditorAssetViewerCursorInSelection()) {
            break;
          }
          // Not viewing an asset.
          const rgg::Texture* texture = rgg::GetTexture(kAssetViewer.texture_id_);
          if (!texture || !texture->IsValid()) break;
          // Cursor isn't in the viewer.
          if (!kCursor.is_in_viewport) break;
          if (kAssetViewerSelection.action == 2) {
            kAssetViewerSelection = {};
            break;
          }
          if (kAssetViewerSelection.action == 0) {
            kAssetViewerSelection.start_texcoord =
                EditorAssetViewerCursorInTexture(*texture);
            kAssetViewerSelection.start_world = EditorAssetViewerCursorWorld();
            kAssetViewerSelection.action = 1;
          }
          else if (kAssetViewerSelection.action == 1) {
            kAssetViewerSelection.end_texcoord =
                EditorAssetViewerCursorInTexture(*texture);
            kAssetViewerSelection.end_world = EditorAssetViewerCursorWorld();
            kAssetViewerSelection.action = 2;
          }
        } break;
      } break;
    } break;
    case KEY_DOWN: {
      switch (event.key) {
        case KEY_NUMPAD_UP:
        case KEY_ARROW_UP: {
          rgg::Camera* camera = EditorAssetViewerCamera();
          if (camera) {
            camera->position += v2f(0.f, ScaleR32(16.f));
          }
        } break;
        case KEY_NUMPAD_RIGHT:
        case KEY_ARROW_RIGHT: {
          rgg::Camera* camera = EditorAssetViewerCamera();
          if (camera) {
            camera->position += v2f(ScaleR32(16.f), 0.f);
          }
        } break;
        case KEY_NUMPAD_DOWN:
        case KEY_ARROW_DOWN: {
          rgg::Camera* camera = EditorAssetViewerCamera();
          if (camera) {
            camera->position += v2f(0.f, ScaleR32(-16.f));
          }
        } break;
        case KEY_NUMPAD_LEFT:
        case KEY_ARROW_LEFT: {
          rgg::Camera* camera = EditorAssetViewerCamera();
          if (camera) {
            camera->position += v2f(ScaleR32(-16.f), 0.f);
          }
        } break;
      }
    } break;
  }
}

// Just a way to verify lines work with the viewport, etc.
void EditorAssetViewerDebugLines() {
  const Rectf& view_rect = ScaleEditorViewport();
  rgg::RenderLine(view_rect.BottomLeft(), view_rect.TopRight(), rgg::kGreen);
  rgg::RenderLine(view_rect.TopLeft(), view_rect.BottomRight(), rgg::kBlue);
  rgg::RenderLineRectangle(view_rect, rgg::kRed);
}

void EditorAssetViewerInitialize() {
  static bool do_once = true;
  if (!do_once) {
    return;
  }
  kAssetViewer.Initialize(kEditor.render_viewport.width, kEditor.render_viewport.height);
  do_once = false;
}

void EditorAssetViewerMain() {
  EditorAssetViewerInitialize();
  kAssetViewer.Render();
  if (kAssetViewerAnimator.is_running_) {
    kAssetViewerAnimator.Render();
  }

  if (kAssetViewerSelection.action == 2) {
    Rectf selection_rect_scaled = kAssetViewerSelection.WorldRectScaled();
    AssetViewerFrame frame = AssetViewerFrame::Create(kAssetViewer.texture_id_, selection_rect_scaled);
    kAssetViewerSelection.action = 0;
    kAssetViewer.frames_.push_back(frame);
  }

  for (std::vector<AssetViewerFrame>::iterator itr = kAssetViewer.frames_.begin();
       itr != kAssetViewer.frames_.end(); ) {
    itr->Render();
    if (itr->is_running_) {
      ++itr;
      continue;
    }
    kAssetViewer.frames_.erase(itr);
  }
}

void EditorAssetViewerDebug() {
  const rgg::Texture* texture = rgg::GetTexture(kAssetViewer.texture_id_);
  if (texture && texture->IsValid()) {
    ImGui::Text("File (%s)", filesystem::Filename(texture->file).c_str());
    ImGui::Text("  texture id    %u", kAssetViewer.texture_id_);
    ImGui::Text("  dims          %.0f %.0f", texture->width, texture->height);
    v2f cursor_in_texture = EditorAssetViewerCursorInTexture(*texture);
    ImGui::Text("  texcoord      %.2f %.2f", cursor_in_texture.x, cursor_in_texture.y);
    ImGui::NewLine();
  }
  ImGui::SliderFloat("scale", &kAssetViewer.scale_, 1.f, 15.f, "%.0f", ImGuiSliderFlags_None);
  ImGui::Checkbox("clamp cursor to nearest edge", &kAssetViewer.clamp_cursor_to_nearest_);
  ImGui::Checkbox("render crosshair", &kAssetViewer.show_crosshair_);
  bool pre_is_animate_running = kAssetViewerAnimator.is_running_;
  ImGui::Checkbox("animate frames", &kAssetViewerAnimator.is_running_);
  bool should_init_animator = pre_is_animate_running != kAssetViewerAnimator.is_running_;
  if (should_init_animator) {
    if (!kAssetViewer.frames_.empty()) {
      const AssetViewerFrame* cf = &kAssetViewer.frames_[0];
      // TODO: This assumes all the frames are the same size. I think that's ok for now?
      kAssetViewerAnimator.Initialize(cf->frame_.dest_rect_.width, cf->frame_.dest_rect_.height);
    }
    else {
      // No frames selected for animating.
      kAssetViewerAnimator.is_running_ = false;
      LOG(WARN, "Tried to run animator with no frames.");
    }
  }

  ImGui::NewLine();
  EditorDebugMenuGrid();
}

r32 EditorAssetViewerScale() {
  return kAssetViewer.scale_;
}
