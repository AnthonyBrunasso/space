#pragma once

class MapMaker : public EditorRenderTarget {
public:
  void OnInitialize() override;
  void OnRender() override;
  void OnImGui() override;
  void OnFileSelected(const std::string& filename) override;

  void ChangeScale(r32 delta);

  void SetNextLayer();
  void SetPrevLayer();

  const Layer2d& GetCurrentLayer() const { return map_.GetLayer(current_layer_); }
  bool HasLayers() const { return map_.HasLayers(); }
  s32 current_layer() const { return current_layer_; }

  Map2d map_;
  s32 current_layer_ = 0;

  bool render_grid_ = true;
  bool render_bounds_ = true;
};

static MapMaker kMapMaker;

class MapMakerControl : public EditorRenderTarget {
public:
  void OnRender() override;
  void OnImGui() override;
  void OnFileSelected(const std::string& filename) override;

  void SetupRenderTarget();
  void SaveToFile(const proto::Map2d& map, const char* filename);
  const rgg::Texture* LoadTexture(const char* tname);
  const rgg::Texture* GetTexture() const;

  void SetSelectionRect(const Rectf& selection) { selection_ = selection; }
  bool HasSelection() const { return selection_.width > 0.f && selection_.height > 0.f; }

  Rectf selection() const { return selection_; }

  enum Mode {
    kMapMakerModeArt = 0,
    kMapMakerModeGeometry = 1, 
    kMapMakerModeCount = 2,
  };

  Mode mode() const { return mode_; }

  Mode mode_;
  rgg::TextureId texture_id_;
  Rectf selection_;
};

static MapMakerControl kMapMakerControl;

void MapMaker::OnInitialize() {}

void MapMaker::OnRender() {
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];

  //glClearColor(1.f, 0.f, 0.f, 1.f);

  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(GetCameraRectScaled(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));

  map_.Render(scale_);
  map_.RenderCollisionGeometry(scale_);

  if (kMapMakerControl.HasSelection() &&
      kMapMakerControl.mode() == MapMakerControl::kMapMakerModeArt) {
    rgg::TextureId texture_id = kMapMakerControl.texture_id_;
    const rgg::Texture* texture = rgg::GetTexture(texture_id);
    if (texture) {
      Rectf src_rect = kMapMakerControl.selection();
      Rectf dest_rect = kMapMaker.cursor().world_grid_cell;
      dest_rect.width = src_rect.width;
      dest_rect.height = src_rect.height;
      rgg::RenderTexture(*texture, src_rect, Scale(dest_rect));
    }
  }

  if (render_grid_) {
    RenderGrid(v4f(1.f, 1.f, 1.f, 0.2f));
    RenderAxis();
  }

  if (kMapMakerControl.mode() == MapMakerControl::kMapMakerModeGeometry) {
    rgg::RenderLineRectangle(Scale(kMapMaker.cursor().world_grid_cell), rgg::kRed);
  }

  if (render_bounds_ && HasLayers()) {
    const Layer2d& current_layer = GetCurrentLayer();
    Rectf render_rect;
    render_rect.x = current_layer.Dims().x / -2.f;
    render_rect.y = current_layer.Dims().y / -2.f;
    render_rect.width = current_layer.Dims().x;
    render_rect.height = current_layer.Dims().y;
    rgg::RenderLineRectangle(Scale(render_rect), kImGuiDebugItemColor);
  }
}

void MapMaker::OnImGui() {
  UpdateImguiPanelRect();
  ImGuiImage();
}

void MapMaker::OnFileSelected(const std::string& filename) {
  if (filesystem::HasExtension(filename.c_str(), "map")) {
    Map2d::LoadFromProtoFile(filename.c_str(), &map_);
  } else {
    kMapMakerControl.OnFileSelected(filename);
  }
}

void MapMaker::ChangeScale(r32 delta) {
  if (scale_ + delta > 0.f && scale_ + delta <= 15.f)
    scale_ += delta;
}

void MapMaker::SetNextLayer() {
  current_layer_++;
  current_layer_ = current_layer_ % map_.GetLayerCount();
}

void MapMaker::SetPrevLayer() {
  current_layer_--;
  if (current_layer_ < 0) current_layer_ = map_.GetLayerCount() - 1;
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
    float pre_scale = scale_;
    if (ImGui::Button("-##scale")) {
      scale_ -= 1.f;
      if (scale_ <= 1.f) scale_ = 1.f;
    }
    ImGui::SameLine();
    if (ImGui::Button("+##scale")) {
      scale_ += 1.f;
      if (scale_ >= 15.f) scale_ = 15.f;
    }
    ImGui::SameLine();
    ImGui::SliderFloat("scale", &scale_, 1.f, 15.f, "%.0f", ImGuiSliderFlags_None);
    if (scale_ != pre_scale) {
      SetupRenderTarget();
    }
  }
  static const char* kMapMakerControlStr[] = {
    "art",
    "geometry"
  };
  if (ImGui::Button("<##mode")) {
    kMapMakerControl.mode_ = (Mode)((s32)kMapMakerControl.mode_ - 1);
    if (kMapMakerControl.mode_ < 0) kMapMakerControl.mode_ = (Mode)((s32)kMapMakerModeCount - 1);
  }
  ImGui::SameLine();
  if (ImGui::Button(">##mode")) {
    kMapMakerControl.mode_ = (Mode)((s32)kMapMakerControl.mode_ + 1);
    if (kMapMakerControl.mode_ >= kMapMakerModeCount) kMapMakerControl.mode_ = (Mode)0;
  }
  ImGui::SameLine();
  ImGui::Combo("mode", (s32*)&kMapMakerControl.mode_, kMapMakerControlStr, 2);
  if (texture) {
    if (ImGui::Button("select all")) {
      SetSelectionRect(texture->Rect());
    }
    ImGui::SameLine();
  }
  static bool kAddLayer = false;
  if (ImGui::Button("add layer")) {
    kAddLayer = true;
  }
  if (kAddLayer) {
    ImGui::Begin("Create Layer");
    static char kWidth[64];
    static char kHeight[64];
    ImGui::InputText("width", kWidth, 128); 
    ImGui::InputText("height", kHeight, 128); 
    r32 width = atof(kWidth);
    r32 height = atof(kHeight);
    ImGui::Text("grid: %.1fx%.1f", width / 16.f, height / 16.f);
    if (ImGui::Button("add")) {
      assert(width > 0.f);
      assert(height > 0.f);
      Rectf new_layer_rect;
      new_layer_rect.x = -width / 2.f;
      new_layer_rect.y = -height / 2.f;
      new_layer_rect.width = width;
      new_layer_rect.height = height;
      kMapMaker.map_.AddLayer(new_layer_rect);
      kAddLayer = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("cancel")) {
      kAddLayer = false;
    }
    ImGui::End();
  }
  ImGui::Separator();
  static char kPngFilename[128];
  static char kFullPath[256];
  ImGui::InputText("file", kPngFilename, 128); 
  snprintf(kFullPath, 256, "gamedata/maps/%s.map", kPngFilename);
  ImGui::Text("%s", kFullPath);
  if (ImGui::Button("save")) {
    //rgg::SaveSurface(kMapMaker.map_.GetSurface(0), kFullPath);
    proto::Map2d proto = kMapMaker.map_.ToProto(kPngFilename);
    SaveToFile(proto, kFullPath);
  }
  ImGui::SameLine();
  if (ImGui::Button("load")) {
    kMapMaker.OnFileSelected(std::string(kFullPath));
  }
  ImGui::SameLine();
  if (ImGui::Button("clear")) {
    kMapMaker.map_.Clear();
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

void MapMakerControl::SaveToFile(const proto::Map2d& map, const char* filename) {
  LOG(INFO, "Saving %s to %s", map.DebugString().c_str(), filename);
  s32 i = 0;
  for (const proto::Layer2d& proto_layer : map.layers()) {
    LOG(INFO, "Saving layer to png: %s", proto_layer.image_file().c_str());
    const Layer2d& layer = kMapMaker.map_.GetLayer(i++);
    rgg::SaveSurface(layer.GetSurface(), proto_layer.image_file().c_str());
  }
  std::fstream fo(filename, std::ios::binary | std::ios::out);
  map.SerializeToOstream(&fo);
  fo.close();
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
            if (kMapMakerControl.HasSelection() &&
                kMapMakerControl.mode() == MapMakerControl::kMapMakerModeArt) {
              const rgg::Texture* texture = rgg::GetTexture(kMapMakerControl.texture_id_);
              assert(texture);
              Rectf src_rect = kMapMakerControl.selection();
              Rectf dest_rect = kMapMaker.cursor().world_grid_cell;
              dest_rect.width = src_rect.width;
              dest_rect.height = src_rect.height;
              kMapMaker.map_.AddTexture(kMapMaker.current_layer(), texture, src_rect, dest_rect);
            }

            if (kMapMakerControl.mode() == MapMakerControl::kMapMakerModeGeometry) {
              Rectf world_rect = kMapMaker.cursor().world_grid_cell;
              kMapMaker.map_.AddGeometry(world_rect);
            }
          }
        } break;
        case BUTTON_RIGHT: {
          if (kMapMakerControl.mode() == MapMakerControl::kMapMakerModeGeometry) {
            kMapMaker.map_.DeleteGeometryAtPoint(kMapMaker.cursor().world);
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
  ImGui::Checkbox("render bounds", &kMapMaker.render_bounds_);
  EditorDebugMenuGrid(kMapMaker.grid());
  if (kMapMaker.HasLayers()) {
    const Layer2d& layer = kMapMaker.GetCurrentLayer();
    ImGui::Text("layer %i / %i", kMapMaker.current_layer() + 1, kMapMaker.map_.GetLayerCount());
    const rgg::Texture& texture = kMapMaker.map_.GetTexture(kMapMaker.current_layer());
    ImGui::Image((void*)(intptr_t)texture.reference, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    ImGuiRenderLastItemBoundingBox();
    ImGui::Text("%.0fx%.0f", layer.Dims().x, layer.Dims().y);
  }
  if (ImGui::Button("<")) {
    kMapMaker.SetPrevLayer();
  }
  ImGui::SameLine();
  if (ImGui::Button(">")) {
    kMapMaker.SetNextLayer();
  }
  ImGui::SameLine();
}
