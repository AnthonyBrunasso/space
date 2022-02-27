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

void EditorAssetViewerDrawAxis(v2f origin_world) {
  const Rectf& view_rect = EditorViewportToWorld();
  rgg::RenderLine(v2f(origin_world.x, view_rect.Min().y),
                  v2f(origin_world.x, view_rect.Max().y),
                  v4f(0.f, 1.f, 0.f, 0.5f));
  rgg::RenderLine(v2f(view_rect.Min().x, origin_world.y),
                  v2f(view_rect.Max().x, origin_world.y),
                  v4f(0.f, 0.f, 1.f, 0.5f));
}

v2f EditorAssetViewerCursorInTexture(const rgg::Texture& texture) {
  v2f viewport_world_to_texture = kCursor.viewport_world + (texture.Rect().Dims() / 2.0);
  return math::Roundf(viewport_world_to_texture);
}

void EditorAssetViewerRenderAsset() {
  const rgg::Texture& tex = kAssetViewer.texture_asset;
  if (!tex.IsValid()) return;
  Rectf dest = EditorRectToWorld(tex.Rect());
  rgg::RenderTexture(tex, tex.Rect(), dest);
}

// Just a way to verify lines work with the viewport, etc.
void EditorAssetViewerDebugLines() {
  const Rectf& view_rect = EditorViewportToWorld();
  rgg::RenderLine(view_rect.BottomLeft(), view_rect.TopRight(), rgg::kGreen);
  rgg::RenderLine(view_rect.TopLeft(), view_rect.BottomRight(), rgg::kBlue);
  rgg::RenderLineRectangle(view_rect, rgg::kRed);
}

// Could alternatively render this to a texture once and use that for the lifespan of the asset
// that's being viewed.
void EditorAssetViewerDrawGrid(v2f start_world, r32 space_world, v4f color) {
  const Rectf& view_rect = EditorViewportToWorld();
  // Draw lines right
  for (r32 start_x = start_world.x; start_x <= view_rect.Max().x; start_x += space_world) {
    rgg::RenderLine(v2f(start_x, view_rect.Min().y), v2f(start_x, view_rect.Max().y), color);
  }
  // Draw lines left
  for (r32 start_x = start_world.x - space_world; start_x >= view_rect.Min().x; start_x -= space_world) {
    rgg::RenderLine(v2f(start_x, view_rect.Min().y), v2f(start_x, view_rect.Max().y), color);
  }
  // Draw lines up
  for (r32 start_y = start_world.y; start_y <= view_rect.Max().y; start_y += space_world) {
    rgg::RenderLine(v2f(view_rect.Min().x, start_y), v2f(view_rect.Max().x, start_y), color);
  }
  // Draw lines down
  for (r32 start_y = start_world.y - space_world; start_y >= view_rect.Min().y; start_y -= space_world) {
    rgg::RenderLine(v2f(view_rect.Min().x, start_y), v2f(view_rect.Max().x, start_y), color);
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
  rgg::RenderRectangle(EditorViewportToWorld(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));
  EditorAssetViewerRenderAsset();

  if (kAssetViewer.texture_asset.IsValid() && kAssetViewer.is_grid_to_texture) {
    v2f origin_world = Vec2ToWorld(EditorAssetViewerTextureBottomLeft(kAssetViewer.texture_asset));
    EditorAssetViewerDrawGrid(origin_world, R32ToWorld(64), v4f(1.f, 1.f, 1.f, 0.2f));
    EditorAssetViewerDrawGrid(origin_world, R32ToWorld(32), v4f(1.f, 1.f, 1.f, 0.1f));
    EditorAssetViewerDrawGrid(origin_world, R32ToWorld(16), v4f(1.f, 1.f, 1.f, 0.05f));
    EditorAssetViewerDrawAxis(origin_world);
  }
  else {
    EditorAssetViewerDrawGrid(v2f(0.f, 0.f), R32ToWorld(64), v4f(1.f, 1.f, 1.f, 0.2f));
    EditorAssetViewerDrawGrid(v2f(0.f, 0.f), R32ToWorld(32), v4f(1.f, 1.f, 1.f, 0.1f));
    EditorAssetViewerDrawGrid(v2f(0.f, 0.f), R32ToWorld(16), v4f(1.f, 1.f, 1.f, 0.05f));
  }

  Rectf view = EditorViewportToWorld();
  if (kCursor.is_in_viewport) {
    if (kAssetViewer.clamp_cursor_to_nearest) {
      v2f unscaled_clamp = Roundf(kCursor.viewport_world_clamped * kAssetViewer.scale);
      rgg::RenderLine(v2f(view.Min().x, unscaled_clamp.y),
                      v2f(view.Max().x, unscaled_clamp.y), v4f(1.f, 0.f, 0.f, 1.f));
      rgg::RenderLine(v2f(unscaled_clamp.x, view.Min().y),
                      v2f(unscaled_clamp.x, view.Max().y), v4f(1.f, 0.f, 0.f, 1.f));

    }
    else {
      rgg::RenderLine(v2f(view.Min().x, kCursor.viewport_world_unscaled.y),
                      v2f(view.Max().x, kCursor.viewport_world_unscaled.y), v4f(1.f, 0.f, 0.f, 1.f));
      rgg::RenderLine(v2f(kCursor.viewport_world_unscaled.x, view.Min().y),
                      v2f(kCursor.viewport_world_unscaled.x, view.Max().y), v4f(1.f, 0.f, 0.f, 1.f));
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
    Rectf wrect = EditorRectToWorld(kAssetViewer.texture_asset.Rect());
    ImGui::Text("  origin     %.2f %.2f", wrect.x, wrect.y);
    ImGui::NewLine();
    ImGui::SliderFloat("scale", &kAssetViewer.scale, 1.f, 5.f, "%.1f", ImGuiSliderFlags_None);
    ImGui::Checkbox("orient grid to texture", &kAssetViewer.is_grid_to_texture);
    ImGui::Checkbox("clamp cursor to nearest edge", &kAssetViewer.clamp_cursor_to_nearest);
    ImGui::NewLine();
  }
  
}

rgg::Camera* EditorAssetViewerCamera() {
  return rgg::CameraGet(kAssetViewer.camera_index);
}

r32 EditorAssetViewerScale() {
  return kAssetViewer.scale;
}
