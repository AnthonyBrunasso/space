#pragma once

static const std::vector<std::string> kEditorKnownAssetExtensions = {
  "tga",
};

bool EditorCanLoadAsset(const std::string& name) {
  for (const std::string& ext : kEditorKnownAssetExtensions) {
    if (filesystem::HasExtension(name.c_str(), ext.c_str())) return true;
  }
  return false;
}

struct AssetViewer {
  rgg::Texture render_target;
  rgg::Texture texture_asset;
  r32 scale = 1.0f;
  u32 camera_index;
  std::string chosen_asset_path;
  bool clamp_cursor_to_nearest = false;
  bool show_crosshair = true;
};

static AssetViewer kAssetViewer;

v2f EditorAssetViewerTextureBottomLeft(const rgg::Texture& tex) {
  return v2f(-tex.width / 2.f, -tex.height / 2.f);
}

// Scale the value given our scale value
r32 __sval(r32 val) {
  return val * kAssetViewer.scale;
}

v2f __svec(v2f vec) {
  return vec * kAssetViewer.scale;
}

v2f EditorAssetViewerCursorInTexture(const rgg::Texture& texture) {
  v2f world_to_texture;
  if (kAssetViewer.clamp_cursor_to_nearest) {
    world_to_texture = kCursor.world_clamped + (texture.Rect().Dims() / 2.0);
  }
  else {
    world_to_texture = kCursor.world + (texture.Rect().Dims() / 2.0);
  }
  return math::Roundf(world_to_texture);
}

v2f EditorAssetViewerCursorWorld() {
  if (kAssetViewer.clamp_cursor_to_nearest) {
    return kCursor.world_clamped;
  }
  return kCursor.world;
}

bool EditorAssetViewerCursorInSelection() {
  return kAssetViewerSelection.action == 2 &&
              math::PointInRect(kCursor.global_screen, kAssetViewerSelection.viewport);
}

rgg::Camera* EditorAssetViewerCamera() {
  return rgg::CameraGet(kAssetViewer.camera_index);
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
          if (!kAssetViewer.texture_asset.IsValid()) break;
          // Cursor isn't in the viewer.
          if (!kCursor.is_in_viewport) break;
          if (kAssetViewerSelection.action == 2) {
            kAssetViewerSelection = {};
            break;
          }
          if (kAssetViewerSelection.action == 0) {
            kAssetViewerSelection.start_texcoord =
                EditorAssetViewerCursorInTexture(kAssetViewer.texture_asset);
            kAssetViewerSelection.start_world = EditorAssetViewerCursorWorld();
            kAssetViewerSelection.action = 1;
          }
          else if (kAssetViewerSelection.action == 1) {
            kAssetViewerSelection.end_texcoord =
                EditorAssetViewerCursorInTexture(kAssetViewer.texture_asset);
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

void EditorAssetViewerDrawAxis(v2f origin_scaled) {
  const Rectf& view_rect = ScaleEditorViewport();
  rgg::RenderLine(v2f(origin_scaled.x, view_rect.Min().y),
                  v2f(origin_scaled.x, view_rect.Max().y),
                  v4f(0.f, 1.f, 0.f, 0.5f));
  rgg::RenderLine(v2f(view_rect.Min().x, origin_scaled.y),
                  v2f(view_rect.Max().x, origin_scaled.y),
                  v4f(0.f, 0.f, 1.f, 0.5f));
}

void EditorAssetViewerRenderAsset() {
  const rgg::Texture& tex = kAssetViewer.texture_asset;
  if (!tex.IsValid()) return;
  Rectf dest = ScaleEditorRect(tex.Rect());
  rgg::RenderTexture(tex, tex.Rect(), dest);
}

// Just a way to verify lines work with the viewport, etc.
void EditorAssetViewerDebugLines() {
  const Rectf& view_rect = ScaleEditorViewport();
  rgg::RenderLine(view_rect.BottomLeft(), view_rect.TopRight(), rgg::kGreen);
  rgg::RenderLine(view_rect.TopLeft(), view_rect.BottomRight(), rgg::kBlue);
  rgg::RenderLineRectangle(view_rect, rgg::kRed);
}

r32 __get_grid_line_color(s32 alpha_num, s32 alpha_1, s32 alpha_2, s32 alpha_3) {
  if (alpha_num == alpha_1) return .1f;
  if (alpha_num == alpha_2) return .2f;
  if (alpha_num == alpha_3) return .4f;
  return .1f;
}

void EditorAssetViewerDrawGrid(v2f start_scaled, const EditorGrid& grid, v4f color) {
  const Rectf& view_rect = ScaleEditorViewport();
  s32 scaled_width = ScaleS32(grid.cell_width);
  s32 scaled_height = ScaleS32(grid.cell_height);
  s32 alpha_1_width = scaled_width;
  s32 alpha_2_width = alpha_1_width * 2;
  s32 alpha_3_width = alpha_2_width * 2;
  s32 alpha_1_height = scaled_height;
  s32 alpha_2_height = alpha_1_height * 2;
  s32 alpha_3_height = alpha_2_height * 2;
  // Draw lines right
  s32 alpha_num = 0;
  for (r32 start_x = start_scaled.x; start_x <= view_rect.Max().x; start_x += scaled_width) {
    color.w = __get_grid_line_color(alpha_num, alpha_1_width, alpha_2_width, alpha_3_width);
    rgg::RenderLine(v2f(start_x, view_rect.Min().y), v2f(start_x, view_rect.Max().y), color);
    alpha_num += scaled_width;
    if (alpha_num > alpha_3_width) alpha_num = alpha_1_width;
  }
  // Draw lines left
  alpha_num = alpha_1_width;
  for (r32 start_x = start_scaled.x - scaled_width; start_x >= view_rect.Min().x; start_x -= scaled_width) {
    color.w = __get_grid_line_color(alpha_num, alpha_1_width, alpha_2_width, alpha_3_width);
    rgg::RenderLine(v2f(start_x, view_rect.Min().y), v2f(start_x, view_rect.Max().y), color);
    alpha_num += scaled_width;
    if (alpha_num > alpha_3_width) alpha_num = alpha_1_width;
  }
  // Draw lines up
  alpha_num = 0;
  for (r32 start_y = start_scaled.y; start_y <= view_rect.Max().y; start_y += scaled_height) {
    color.w = __get_grid_line_color(alpha_num, alpha_1_height, alpha_2_height, alpha_3_height);
    rgg::RenderLine(v2f(view_rect.Min().x, start_y), v2f(view_rect.Max().x, start_y), color);
    alpha_num += scaled_height;
    if (alpha_num > alpha_3_height) alpha_num = alpha_1_height;
  }
  // Draw lines down
  alpha_num = alpha_1_height;
  for (r32 start_y = start_scaled.y - scaled_height; start_y >= view_rect.Min().y; start_y -= scaled_height) {
    color.w = __get_grid_line_color(alpha_num, alpha_1_height, alpha_2_height, alpha_3_height);
    rgg::RenderLine(v2f(view_rect.Min().x, start_y), v2f(view_rect.Max().x, start_y), color);
    alpha_num += scaled_height;
    if (alpha_num > alpha_3_height) alpha_num = alpha_1_height;
  }
}

void EditorAssetViewerDrawCrosshair(v2f point_scaled, v4f color = rgg::kRed) {
  Rectf view = ScaleEditorViewport();
  rgg::RenderLine(v2f(view.Min().x, point_scaled.y),
                  v2f(view.Max().x, point_scaled.y), color);
  rgg::RenderLine(v2f(point_scaled.x, view.Min().y),
                  v2f(point_scaled.x, view.Max().y), color);
}


void EditorAssetViewerInit() {
  static bool kInitOnce = true;
  if (!kInitOnce) {
    return;
  }
  kAssetViewer.render_target =
      rgg::CreateEmptyTexture2D(GL_RGB, kEditorState.render_viewport.width, kEditorState.render_viewport.height);
  rgg::Camera camera;
  camera.position = v3f(0.f, 0.f, 0.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraBrowser;
  camera.speed = v3f(5.f, 5.f, 0.0f);
  camera.viewport = kEditorState.render_viewport.Dims();
  camera.camera_control = false;
  kAssetViewer.camera_index = rgg::CameraInit(camera);
  kInitOnce = false;
}

void EditorAssetViewerUpdateSelectionTexture(rgg::Texture* texture, const Rectf& rect) {
  if (!texture) return;
  if (texture->width != rect.width || texture->height != rect.height) {
    rgg::DestroyTexture2D(texture);
    *texture = rgg::CreateEmptyTexture2D(GL_RGB, rect.width, rect.height);
  }
  Rectf texrect = kAssetViewerSelection.TexRect();
  rgg::BeginRenderTo(*texture);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  rgg::RenderTexture(
      kAssetViewer.texture_asset,
      texrect, Rectf(-rect.width, -rect.height, rect.width * 2, rect.height * 2));
  rgg::EndRenderTo();
}

void EditorAssetViewerMain() {
  if (!kAssetViewer.chosen_asset_path.empty()) {
    rgg::DestroyTexture2D(&kAssetViewer.texture_asset);
    rgg::TextureInfo texture_info;
    texture_info.min_filter = GL_NEAREST_MIPMAP_NEAREST;
    texture_info.mag_filter = GL_NEAREST;
    if (!rgg::LoadTGA(kAssetViewer.chosen_asset_path.c_str(),
                      texture_info, &kAssetViewer.texture_asset)) {
      LOG(WARN, "Unable to load asset %s", kAssetViewer.chosen_asset_path.c_str());
    }
    kGrid.origin = EditorAssetViewerTextureBottomLeft(kAssetViewer.texture_asset);
    kAssetViewer.chosen_asset_path.clear();
  }

  EditorAssetViewerInit();

  rgg::CameraSwitch(kAssetViewer.camera_index);
  rgg::Camera* c = rgg::CameraGetCurrent(); 
  rgg::ModifyObserver mod(
      math::Ortho(kEditorState.render_viewport.width, 0.f,
                  kEditorState.render_viewport.height, 0.f, -1.f, 1.f),
      math::LookAt(c->position, c->position + v3f(0.f, 0.f, 1.f),
                   v3f(0.f, -1.f, 0.f)));
  rgg::BeginRenderTo(kAssetViewer.render_target);
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(ScaleEditorViewport(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));
  EditorAssetViewerRenderAsset();

  v2f origin_scaled = ScaleVec2(v2f(0.f, 0.f));
  if (kAssetViewer.texture_asset.IsValid()) {
    origin_scaled = ScaleVec2(kGrid.origin);
  }

  EditorAssetViewerDrawGrid(origin_scaled, kGrid, v4f(1.f, 1.f, 1.f, 0.2f));
  EditorAssetViewerDrawAxis(origin_scaled);

  // Useful for debugging cursor stuff
  //rgg::RenderLine(kCursor.world, v2f(0.f, 0.f), rgg::kWhite);

  if (kCursor.is_in_viewport && kAssetViewer.show_crosshair && !EditorAssetViewerCursorInSelection()) {
    if (kAssetViewer.clamp_cursor_to_nearest) {
      v2f scaled_clamp = kCursor.world_clamped * kAssetViewer.scale;
      EditorAssetViewerDrawCrosshair(scaled_clamp);
    }
    else {
      EditorAssetViewerDrawCrosshair(kCursor.world_scaled);
    }
  }

  if (kAssetViewerSelection.action) {
    if (kAssetViewerSelection.action == 1) {
      EditorAssetViewerDrawCrosshair(kAssetViewerSelection.start_world * kAssetViewer.scale, rgg::kPurple);
    }
    else if (kAssetViewerSelection.action == 2) {
      EditorAssetViewerDrawCrosshair(kAssetViewerSelection.start_world * kAssetViewer.scale, rgg::kPurple);
      EditorAssetViewerDrawCrosshair(kAssetViewerSelection.end_world * kAssetViewer.scale, rgg::kPurple);
    }
  }

  rgg::EndRenderTo();
  ImGui::Image((void*)(intptr_t)kAssetViewer.render_target.reference,
               ImVec2(kEditorState.render_viewport.width, kEditorState.render_viewport.height));

  if (kAssetViewerSelection.action == 2) {
    static bool kAssetSelectionOpen = true;
    ImGui::Begin("Asset Selection", &kAssetSelectionOpen);
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    kAssetViewerSelection.viewport.x = pos.x;
    v2f wsize = window::GetWindowSize();
    kAssetViewerSelection.viewport.y = wsize.y - pos.y - size.y;
    kAssetViewerSelection.viewport.width = size.x;
    // Imgui uses sadness top left of screen as origin.
    kAssetViewerSelection.viewport.height = size.y;
    Rectf rect = kAssetViewerSelection.WorldRectScaled();
    static rgg::Texture subtexture = rgg::CreateEmptyTexture2D(GL_RGB, rect.width, rect.height);
    EditorAssetViewerUpdateSelectionTexture(&subtexture, rect);
    ImGui::Image((void*)(intptr_t)subtexture.reference, ImVec2(rect.width, rect.height));
    ImGui::End();
  }
}

void EditorAssetViewerDebug() {
  if (kAssetViewer.texture_asset.IsValid()) {
    ImGui::Text("File (%s)", filesystem::Filename(kAssetViewer.texture_asset.file).c_str());
    ImGui::Text("  dims          %.0f %.0f", kAssetViewer.texture_asset.width, kAssetViewer.texture_asset.height);
    v2f cursor_in_texture = EditorAssetViewerCursorInTexture(kAssetViewer.texture_asset);
    ImGui::Text("  texcoord      %.2f %.2f", cursor_in_texture.x, cursor_in_texture.y);
    ImGui::Text("  select start  %.2f %.2f",
                kAssetViewerSelection.start_texcoord.x,
                kAssetViewerSelection.start_texcoord.y);
    ImGui::Text("  select end    %.2f %.2f",
                kAssetViewerSelection.end_texcoord.x,
                kAssetViewerSelection.end_texcoord.y);
    ImGui::NewLine();
  }
  ImGui::SliderFloat("scale", &kAssetViewer.scale, 1.f, 5.f, "%.0f", ImGuiSliderFlags_None);
  ImGui::Checkbox("clamp cursor to nearest edge", &kAssetViewer.clamp_cursor_to_nearest);
  ImGui::Checkbox("render crosshair", &kAssetViewer.show_crosshair);
  ImGui::NewLine();
  EditorDebugMenuGrid();
}

r32 EditorAssetViewerScale() {
  return kAssetViewer.scale;
}
