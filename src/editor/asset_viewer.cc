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
};

static AssetViewer kAssetViewer;

// Scale the value given our scale value
r32 __sval(r32 val) {
  return val * kAssetViewer.scale;
}

void EditorAssetViewerRenderAsset() {
  const rgg::Texture& tex = kAssetViewer.texture_asset;
  if (!tex.IsValid()) return;
  Rectf dest = tex.Rect();
  if (kAssetViewer.scale != 1.f) {
    dest.width = __sval(dest.width);
    dest.height = __sval(dest.height);
  }
  dest.x -= dest.width / 2.f;
  dest.y -= dest.height / 2.f;
  rgg::RenderTexture(tex, tex.Rect(), dest);
}

// Just a way to verify lines work with the viewport, etc.
void EditorAssetViewerDebugLines() {
  const Rectf& view_rect = EditorRenderableViewRect();
  rgg::RenderLine(view_rect.BottomLeft(), view_rect.TopRight(), rgg::kGreen);
  rgg::RenderLine(view_rect.TopLeft(), view_rect.BottomRight(), rgg::kBlue);
  rgg::RenderLineRectangle(view_rect, rgg::kRed);
}

void EditorAssetViewerDrawGrid(r32 space, v4f color) {
  const Rectf& view_rect = EditorRenderableViewRect();
  // Lets draw a line every 64 px from origin to the edge of the viewport
  for (r32 start_x = 0.f; start_x < view_rect.Max().x; start_x += space) {
    rgg::RenderLine(v2f(__sval(start_x), view_rect.Min().y), v2f(__sval(start_x), view_rect.Max().y), color);
    if (start_x > 0) {
      rgg::RenderLine(v2f(__sval(-start_x), view_rect.Min().y), v2f(__sval(-start_x), view_rect.Max().y), color);
    }
  }

  for (r32 start_y = 0.f; start_y < view_rect.Max().y; start_y += space) {
    rgg::RenderLine(v2f(view_rect.Min().x, __sval(start_y)), v2f(view_rect.Max().x, __sval(start_y)), color);
    if (start_y > 0) {
      rgg::RenderLine(v2f(view_rect.Min().x, __sval(-start_y)), v2f(view_rect.Max().x, __sval(-start_y)), color);
    }
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
  camera.viewport = window::GetWindowSize();
  camera.camera_control = false;
  kAssetViewer.camera_index = rgg::CameraInit(camera);
  kInitOnce = false;
}

void EditorAssetViewerMain() {
  if (!kAssetViewer.chosen_asset_path.empty()) {
    rgg::DestroyTexture2D(&kAssetViewer.texture_asset);
    if (!rgg::LoadTGA(kAssetViewer.chosen_asset_path.c_str(),
                      rgg::DefaultTextureInfo(), &kAssetViewer.texture_asset)) {
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
  glClear(GL_COLOR_BUFFER_BIT);
  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(EditorRenderableViewRect(), v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));
  EditorAssetViewerRenderAsset();
  EditorAssetViewerDrawGrid(64, v4f(1.f, 1.f, 1.f, 0.2f));
  EditorAssetViewerDrawGrid(32, v4f(1.f, 1.f, 1.f, 0.1f));
  EditorAssetViewerDrawGrid(16, v4f(1.f, 1.f, 1.f, 0.05f));

  v2f raw_cursor = window::GetCursorPosition();
  v2f cursor_in_view = EditorCursorToRenderView(raw_cursor);
  Rectf view = EditorRenderableViewRect();
  if (math::PointInRect(raw_cursor, kEditorState.render_viewport_in_editor)) {
    rgg::RenderLine(v2f(view.Min().x, cursor_in_view.y),
                    v2f(view.Max().x, cursor_in_view.y), v4f(1.f, 0.f, 0.f, .4f));
  }

  rgg::EndRenderTo();
  ImGui::Begin("Debug");
  //ImGui::Text("%.2f %.2f");
  ImGui::End();
  ImGui::Image((void*)(intptr_t)kAssetViewer.render_target.reference,
               ImVec2(kEditorState.render_viewport.width, kEditorState.render_viewport.height));
}

void EditorAssetViewerDebug() {
  v2f raw_cursor = window::GetCursorPosition();
  v2f cursor_in_view = EditorCursorToRenderView(window::GetCursorPosition());
  if (math::PointInRect(raw_cursor, kEditorState.render_viewport_in_editor)) {
    ImGui::Text("Viewport %.0f %.0f %.0f %.0f", 
      kEditorState.render_viewport_in_editor.x,
      kEditorState.render_viewport_in_editor.y,
      kEditorState.render_viewport_in_editor.width,
      kEditorState.render_viewport_in_editor.height);
    ImGui::Text("cursor_in_view %.2fx%.2f", cursor_in_view.x, cursor_in_view.y);
    ImGui::Text("cursor_raw     %.2fx%.2f", raw_cursor.x, raw_cursor.y);
    ImGui::NewLine();
  }
  if (kAssetViewer.texture_asset.IsValid()) {
    ImGui::Text("File (%s)", filesystem::Filename(kAssetViewer.texture_asset.file).c_str());
    ImGui::Text("  reference %u", kAssetViewer.texture_asset.reference);
    ImGui::Text("  width %.0f height %.0f", kAssetViewer.texture_asset.width, kAssetViewer.texture_asset.height);
    ImGui::NewLine();
    ImGui::SliderFloat("scale", &kAssetViewer.scale, 1.f, 3.f, "%.3f", ImGuiSliderFlags_None);
    ImGui::Text("1.0 -> 3.0");
  }
}
