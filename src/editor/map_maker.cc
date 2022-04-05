#pragma once

class MapMaker : public EditorRenderTarget {
public:
  void OnInitialize() override;
  void OnRender() override;
  void OnImGui() override;
  void OnFileSelected(const std::string& filename) override;

  void ChangeScale(r32 delta);

  void SetNextLayer() { current_layer_++; current_layer_ = current_layer_ % map_.GetLayerCount(); }
  void SetPrevLayer() { current_layer_--; current_layer_ = current_layer_ % map_.GetLayerCount(); }

  s32 current_layer() const { return current_layer_; }

  Map2d map_;
  s32 current_layer_ = 0;

  bool render_grid_ = true;
};

static MapMaker kMapMaker;

class MapMakerControl : public EditorRenderTarget {
public:
  void OnRender() override;
  void OnImGui() override;
  void OnFileSelected(const std::string& filename) override;

  void SetupRenderTarget();
  const rgg::Texture* LoadTexture(const char* tname);
  const rgg::Texture* GetTexture() const;

  void SetSelectionRect(const Rectf& selection) { selection_ = selection; }
  bool HasSelection() const { return selection_.width > 0.f && selection_.height > 0.f; }

  Rectf selection() const { return selection_; }

  rgg::TextureId texture_id_;
  Rectf selection_;
};

static MapMakerControl kMapMakerControl;

void MapMaker::OnInitialize() {
  // TODO: Probably need this to be specified in editor?
  map_ = Map2d(GetRenderTargetDims());
}

void MapMaker::OnRender() {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];

  //glClearColor(1.f, 0.f, 0.f, 1.f);

  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));

  map_.Render(scale_);

  if (kMapMakerControl.HasSelection()) {
    rgg::TextureId texture_id = kMapMakerControl.texture_id_;
    const rgg::Texture* texture = rgg::GetTexture(texture_id);
    if (texture) {
      Rectf src_rect = kMapMakerControl.selection();
      Rectf dest_rect = kMapMaker.cursor().world_grid_cell;
      dest_rect.width = src_rect.width;
      dest_rect.height = src_rect.height;
      dest_rect.x *= scale_;
      dest_rect.y *= scale_;
      dest_rect.width *= scale_;
      dest_rect.height *= scale_;
      rgg::RenderTexture(*texture, src_rect, dest_rect);
    }
  }

  if (render_grid_) {
    RenderGrid(v4f(1.f, 1.f, 1.f, 0.2f));
    RenderAxis();
  }
}

void MapMaker::OnImGui() {
  UpdateImguiPanelRect();
  ImGuiImage();
}

void MapMaker::OnFileSelected(const std::string& filename) {
  kMapMakerControl.OnFileSelected(filename);
}

void MapMaker::ChangeScale(r32 delta) {
  if (scale_ + delta > 0.f && scale_ + delta <= 15.f)
    scale_ += delta;
}

void MapMakerControl::OnRender() {
  const rgg::Texture* texture = rgg::GetTexture(texture_id_);
  if (texture) {
    grid_.origin = (GetRenderTargetDims() * scale_) / -2.f;
    grid_.origin_offset = v2f(0.f, 0.f);
  }

  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];

  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));

  rgg::RenderTexture(*texture, texture->Rect(), GetCameraRectScaled());

  RenderGrid(v4f(1.f, 1.f, 1.f, 0.2f));
  RenderCursorAsRect();
}

void MapMakerControl::OnImGui() {
  ImGui::Begin("Map Maker Control", nullptr, ImGuiWindowFlags_HorizontalScrollbar);
  ImGuiImage();
  const rgg::Texture* texture = rgg::GetTexture(texture_id_);
  if (texture) {
    EditorDebugMenuGrid(&grid_);
    if (selection_.width > 0.f && selection_.height > 0.f) {
      ImGuiTextRect("selection", selection_);
    }
    float pre_scale = scale_;
    ImGui::SliderFloat("scale", &scale_, 1.f, 15.f, "%.0f", ImGuiSliderFlags_None);
    if (scale_ != pre_scale) {
      SetupRenderTarget();
    }
    if (ImGui::Button("select all")) {
      SetSelectionRect(texture->Rect());
    }
  }

  static char kPngFilename[128];
  static char kFullPath[256];
  ImGui::InputText("file", kPngFilename, 128); 
  snprintf(kFullPath, 256, "gamedata/%s.png", kPngFilename);
  ImGui::Text("%s", kFullPath);
  if (ImGui::Button("Save")) {
    rgg::SaveSurface(kMapMaker.map_.GetSurface(0), kFullPath);
  }

  UpdateImguiPanelRect();
  ImGui::End();
}

void MapMakerControl::OnFileSelected(const std::string& filename) {
  LoadTexture(filename.c_str());
  SetupRenderTarget();
}

const rgg::Texture* MapMakerControl::GetTexture() const {
  return rgg::GetTexture(texture_id_);
}

void MapMakerControl::SetupRenderTarget() {
  const rgg::Texture* texture = GetTexture();
  if (!texture) return;
  ReleaseSurface();
  Initialize((s32)texture->width * scale_, (s32)texture->height * scale_);
}

const rgg::Texture* MapMakerControl::LoadTexture(const char* tname) {
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


void EditorMapMakerProcessEvent(const PlatformEvent& event) {
  kMapMaker.UpdateCursor();
  kMapMakerControl.UpdateCursor();

  switch(event.type) {
    case MOUSE_DOWN: {
      switch (event.key) {
        case BUTTON_LEFT: {
          if (kMapMakerControl.IsMouseInside() && kMapMakerControl.IsMouseInsideEditorSurface()) {
            Rectf tex_rect = kMapMakerControl.cursor().world_grid_cell;
            const rgg::Texture* texture = kMapMakerControl.GetTexture();
            if (texture) {
              tex_rect.x += texture->width / 2.f;
              tex_rect.y += texture->height / 2.f;
              kMapMakerControl.SetSelectionRect(tex_rect);
            }
          }

          if (kMapMaker.IsMouseInsideEditorSurface() && !kMapMakerControl.IsMouseInside()) {
            if (kMapMakerControl.HasSelection()) {
              const rgg::Texture* texture = rgg::GetTexture(kMapMakerControl.texture_id_);
              assert(texture);
              Rectf src_rect = kMapMakerControl.selection();
              Rectf dest_rect = kMapMaker.cursor().world_grid_cell;
              dest_rect.width = src_rect.width;
              dest_rect.height = src_rect.height;
              kMapMaker.map_.AddTexture(kMapMaker.current_layer(), texture, src_rect, dest_rect);
            }
          }
        } break;
      } break;
    } break;
    case KEY_DOWN: {
      switch (event.key) {
        case KEY_NUMPAD_UP:
        case KEY_ARROW_UP: {
          rgg::Camera* camera = kMapMaker.camera();
          if (camera) {
            camera->position += v2f(0.f, ScaleR32(16.f));
          }
        } break;
        case KEY_NUMPAD_RIGHT:
        case KEY_ARROW_RIGHT: {
          rgg::Camera* camera = kMapMaker.camera();
          if (camera) {
            camera->position += v2f(ScaleR32(16.f), 0.f);
          }
        } break;
        case KEY_NUMPAD_DOWN:
        case KEY_ARROW_DOWN: {
          rgg::Camera* camera = kMapMaker.camera();
          if (camera) {
            camera->position += v2f(0.f, ScaleR32(-16.f));
          }
        } break;
        case KEY_NUMPAD_LEFT:
        case KEY_ARROW_LEFT: {
          rgg::Camera* camera = kMapMaker.camera();
          if (camera) {
            camera->position += v2f(ScaleR32(-16.f), 0.f);
          }
        } break;
      }
    } break;
    case MOUSE_WHEEL: {
      if (kMapMaker.IsMouseInsideEditorSurface() && !kMapMakerControl.IsMouseInside()) {
        if (event.wheel_delta > 0) {
          kMapMaker.ChangeScale(1.f);
        } else if (event.wheel_delta < 0) {
          kMapMaker.ChangeScale(-1.f);
        }
      }
    } break;
    case MOUSE_UP:
    case NOT_IMPLEMENTED:
    default: break;
  }

}

void EditorMapMakerInitialize() {
  static bool do_once = true;
  if (!do_once) {
    return;
  }
  LOG(INFO, "Creating map maker viewport with %i %i", (s32)kEditor.render_viewport.width, (s32)kEditor.render_viewport.height);
  kMapMaker.Initialize((s32)kEditor.render_viewport.width, (s32)kEditor.render_viewport.height);
  do_once = false;
}

void EditorMapMakerMain() {
  EditorSetCurrent(&kMapMaker);
  EditorMapMakerInitialize();
  kMapMaker.Render();
  kMapMakerControl.Render();
}

void EditorMapMakerDebug() {
  ImGui::Checkbox("render grid", &kMapMaker.render_grid_);
  if (kMapMaker.map_.HasLayers()) {
    ImGui::Text("Layer %i / %i", kMapMaker.current_layer() + 1, kMapMaker.map_.GetLayerCount());
    const rgg::Texture& texture = kMapMaker.map_.GetTexture(kMapMaker.current_layer());
    ImGui::Image((void*)(intptr_t)texture.reference, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    ImGuiRenderLastItemBoundingBox();
  }
  if (ImGui::Button("<")) {
    kMapMaker.SetPrevLayer();
  }
  ImGui::SameLine();
  if (ImGui::Button(">")) {
    kMapMaker.SetNextLayer();
  }
  ImGui::SameLine();
  if (ImGui::Button("+")) {
    kMapMaker.map_.AddLayer();
  }
  
}

rgg::Camera* EditorMapMakerCamera() {
  return nullptr;
}

r32 EditorMapMakerScale() {
  return 1.f;
}
