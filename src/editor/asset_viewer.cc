#pragma once

DEFINE_EDITOR_CALLBACK(AssetBoxSelect, AssetSelection);

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

class AssetViewerAnimator : public EditorRenderTarget {
public:
  AssetViewerAnimator();
  AssetViewerAnimator(const AssetViewerAnimator& anim) = delete;

  static void HandleAssetBoxSelect(const AssetSelection& selection);

  void OnRender() override;
  void OnImGui() override;

  bool IsMouseInside() const override;

  void AddFrame(const AnimFrame2d& frame, v2f dims, r32 duration = 1.f);
  void Clear();

  AnimSequence2d anim_sequence_;
  struct Frame {
    EditorSurface editor_surface;
    Rectf imgui_rect;
    void ImGui(AnimSequence2d::SequenceFrame& sframe, s32 id);
  };

  std::vector<Frame> anim_frames_;
  bool is_running_ = true;
};

static AssetViewerAnimator kAssetViewerAnimator;

class AssetViewer : public EditorRenderTarget {
public:
  void OnRender() override;
  void OnImGui() override;

  rgg::TextureId texture_id_;
  r32 scale_ = 1.0f;
  std::string chosen_asset_path_;
  bool clamp_cursor_to_nearest_ = true;
  bool show_crosshair_ = true;
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
}

void AssetViewer::OnImGui() {
  ImGuiImage();
}

void AssetViewerAnimator::HandleAssetBoxSelect(const AssetSelection& selection) {
  AnimFrame2d frame;
  frame.texture_id_ = kAssetViewer.texture_id_;
  frame.src_rect_ = selection.tex_rect;
  kAssetViewerAnimator.AddFrame(frame, selection.world_rect_scaled.Dims(), 1.f);
  kAssetViewerAnimator.anim_sequence_.Start();
  if (!kAssetViewerAnimator.IsRenderTargetValid()) {
    kAssetViewerAnimator.Initialize(selection.world_rect_scaled.width, selection.world_rect_scaled.height);
  }
}

AssetViewerAnimator::AssetViewerAnimator() {
  SubscribeAssetBoxSelect(&HandleAssetBoxSelect);
}

void AssetViewerAnimator::OnRender() {
  anim_sequence_.Update();

  if (!anim_sequence_.IsEmpty()) {
    //const AssetViewerFrame* cf = &kAssetViewer.frames_[anim_sequence_.frame_index_];
    const AnimFrame2d& aframe = anim_sequence_.CurrentFrame();
    const rgg::Texture* texture_render_from = rgg::GetTexture(aframe.texture_id_);
    rgg::RenderTexture(*texture_render_from,
                       aframe.src_rect(),
                       Rectf(-GetRenderTargetWidth() / 2.f, -GetRenderTargetHeight() / 2.f,
                             GetRenderTargetWidth(), GetRenderTargetHeight()));
  }
}

void AssetViewerAnimator::Clear() {
  anim_sequence_.Clear();
  ReleaseSurface();
  for (Frame& frame : anim_frames_) {
    DestroyEditorSurface(&frame.editor_surface);
  }
  anim_frames_.clear();
}

void AssetViewerAnimator::OnImGui() {
  ImGui::Begin("Animator", &kAssetViewerAnimator.is_running_);
  ImGuiImage();
  UpdateImguiPanelRect();
  // Walk all the frames at a certain cadence and play them.
  //ImGui::SliderFloat("frequency", &frequency_, 0.f, 2.f, "%.01f", ImGuiSliderFlags_None);
  if (ImGui::Button("reset clock")) {
    anim_sequence_.Start();
  }
  ImGui::Text("Clock: %.2f", platform::ClockDeltaSec(anim_sequence_.clock_));
  ImGui::Text("%i %.2f / %.2f",
              anim_sequence_.frame_index_,
              anim_sequence_.last_frame_time_sec_,
              anim_sequence_.next_frame_time_sec_);
  if (ImGui::Button("Generate")) {
    proto::Animation2d proto = anim_sequence_.ToProto();
    LOG(INFO, "%s", proto.DebugString().c_str());
  }
  if (ImGui::Button("Clear")) {
    Clear();
  }
  ImGui::End();

  if (!anim_sequence_.IsEmpty()) {
    assert(anim_sequence_.FrameCount() == anim_frames_.size());
    int i = 0;
    for (AnimSequence2d::SequenceFrame& sequence_frame : anim_sequence_.sequence_frames_) {
      Frame* render_frame = &anim_frames_[i];
      render_frame->ImGui(sequence_frame, i);
      ++i;
    }
  }
}

void AssetViewerAnimator::Frame::ImGui(AnimSequence2d::SequenceFrame& sframe, s32 id) {
  const rgg::Texture* texture = rgg::GetTexture(sframe.frame.texture_id_);
  // At this point if the texture isn't loaded we have a deeper problem. Load textures before animating.
  assert(texture != nullptr);
  char panel_name[128];
  snprintf(panel_name, 128, "%s/%i", filesystem::Filename(texture->file).c_str(), id);

  ImGui::Begin(panel_name);
  GetImGuiPanelRect(&imgui_rect);

  RenderSurfaceToImGuiImage(editor_surface, texture, sframe.frame.src_rect(), sframe.is_active);

  if (ImGui::Button("-##x")) sframe.frame.src_rect_.x += 1.f;
  ImGui::SameLine();
  if (ImGui::Button("+##x")) sframe.frame.src_rect_.x -= 1.f;
  ImGui::SameLine();
  ImGui::Text("x");

  if (ImGui::Button("-##y")) sframe.frame.src_rect_.y += 1.f;
  ImGui::SameLine();
  if (ImGui::Button("+##y")) sframe.frame.src_rect_.y -= 1.f;
  ImGui::SameLine();
  ImGui::Text("y");

  ImGui::Text("t: %.2f", sframe.duration_sec);
  if (ImGui::Button("-1s")) sframe.duration_sec -= 1.f;
  ImGui::SameLine();
  if (ImGui::Button("-.1s")) sframe.duration_sec -= 0.1f;
  ImGui::SameLine();
  if (ImGui::Button("-.01s")) sframe.duration_sec -= 0.01f;

  if (ImGui::Button("+1s")) sframe.duration_sec += 1.f;
  ImGui::SameLine();
  if (ImGui::Button("+.1s")) sframe.duration_sec += 0.1f;
  ImGui::SameLine();
  if (ImGui::Button("+.01s")) sframe.duration_sec += 0.01f;

  // The min we can possibly leave an open frame.
  if (sframe.duration_sec < 0.01f) sframe.duration_sec = 0.01f;

  ImGui::End();
}

bool AssetViewerAnimator::IsMouseInside() const {
  if (EditorRenderTarget::IsMouseInside()) return true;
  for (const Frame& frame : anim_frames_) {
    if (math::PointInRect(kCursor.global_screen, frame.imgui_rect)) return true;
  }
  return false;
}

void AssetViewerAnimator::AddFrame(const AnimFrame2d& frame, v2f dims, r32 duration) {
  anim_sequence_.AddFrame(frame, 1.f);
  Frame asset_frame;
  asset_frame.editor_surface = CreateEditorSurface(dims.x, dims.y);
  asset_frame.imgui_rect = {};
  assert(asset_frame.editor_surface.IsValid());
  anim_frames_.push_back(asset_frame);
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
            AssetSelection selection;
            selection.tex_rect = math::MakeRect(
                kAssetViewerSelection.start_texcoord, kAssetViewerSelection.end_texcoord);
            selection.world_rect = math::MakeRect(
                kAssetViewerSelection.start_world, kAssetViewerSelection.end_world);
            selection.world_rect_scaled = kAssetViewerSelection.WorldRectScaled();
            DispatchAssetBoxSelect(selection);
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
    kAssetViewerSelection.action = 0;
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

  ImGui::NewLine();
  EditorDebugMenuGrid();
}

r32 EditorAssetViewerScale() {
  return kAssetViewer.scale_;
}
