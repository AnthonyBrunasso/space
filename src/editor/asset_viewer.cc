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
  bool is_grid_to_texture = true;
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

void EditorAssetViewerDrawAxis(v2f origin_scaled) {
  const Rectf& view_rect = ScaleEditorViewport();
  rgg::RenderLine(v2f(origin_scaled.x, view_rect.Min().y),
                  v2f(origin_scaled.x, view_rect.Max().y),
                  v4f(0.f, 1.f, 0.f, 0.5f));
  rgg::RenderLine(v2f(view_rect.Min().x, origin_scaled.y),
                  v2f(view_rect.Max().x, origin_scaled.y),
                  v4f(0.f, 0.f, 1.f, 0.5f));
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
  if (kAssetViewer.texture_asset.IsValid() && kAssetViewer.is_grid_to_texture) {
    origin_scaled = ScaleVec2(kGrid.origin);
  }

  EditorAssetViewerDrawGrid(origin_scaled, kGrid, v4f(1.f, 1.f, 1.f, 0.2f));
  EditorAssetViewerDrawAxis(origin_scaled);

  // Useful for debugging cursor stuff
  //rgg::RenderLine(kCursor.world, v2f(0.f, 0.f), rgg::kWhite);

  Rectf view = ScaleEditorViewport();
  if (kCursor.is_in_viewport && kAssetViewer.show_crosshair) {
    if (kAssetViewer.clamp_cursor_to_nearest) {
      v2f scaled_clamp = kCursor.world_clamped * kAssetViewer.scale;
      rgg::RenderLine(v2f(view.Min().x, scaled_clamp.y),
                      v2f(view.Max().x, scaled_clamp.y), v4f(1.f, 0.f, 0.f, 1.f));
      rgg::RenderLine(v2f(scaled_clamp.x, view.Min().y),
                      v2f(scaled_clamp.x, view.Max().y), v4f(1.f, 0.f, 0.f, 1.f));

    }
    else {
      rgg::RenderLine(v2f(view.Min().x, kCursor.world_scaled.y),
                      v2f(view.Max().x, kCursor.world_scaled.y), v4f(1.f, 0.f, 0.f, 1.f));
      rgg::RenderLine(v2f(kCursor.world_scaled.x, view.Min().y),
                      v2f(kCursor.world_scaled.x, view.Max().y), v4f(1.f, 0.f, 0.f, 1.f));
    }
  }

  rgg::EndRenderTo();
  ImGui::Begin("Debug");
  //ImGui::Text("%.2f %.2f");
  ImGui::End();
  ImGui::Image((void*)(intptr_t)kAssetViewer.render_target.reference,
               ImVec2(kEditorState.render_viewport.width, kEditorState.render_viewport.height));
}

void EditorAssetViewerDebug() {
  if (kAssetViewer.texture_asset.IsValid()) {
    ImGui::Text("File (%s)", filesystem::Filename(kAssetViewer.texture_asset.file).c_str());
    ImGui::Text("  reference  %u", kAssetViewer.texture_asset.reference);
    ImGui::Text("  dims       %.0f %.0f", kAssetViewer.texture_asset.width, kAssetViewer.texture_asset.height);
    v2f cursor_in_texture = EditorAssetViewerCursorInTexture(kAssetViewer.texture_asset);
    ImGui::Text("  texcoord   %.2f %.2f", cursor_in_texture.x, cursor_in_texture.y);
    Rectf wrect = ScaleEditorRect(kAssetViewer.texture_asset.Rect());
    ImGui::Text("  origin     %.2f %.2f", wrect.x, wrect.y);
    ImGui::NewLine();
  }
  ImGui::SliderFloat("scale", &kAssetViewer.scale, 1.f, 5.f, "%.0f", ImGuiSliderFlags_None);
  ImGui::Checkbox("orient grid to texture", &kAssetViewer.is_grid_to_texture);
  ImGui::Checkbox("clamp cursor to nearest edge", &kAssetViewer.clamp_cursor_to_nearest);
  ImGui::Checkbox("render crosshair", &kAssetViewer.show_crosshair);
  ImGui::NewLine();
}

rgg::Camera* EditorAssetViewerCamera() {
  return rgg::CameraGet(kAssetViewer.camera_index);
}

r32 EditorAssetViewerScale() {
  return kAssetViewer.scale;
}
