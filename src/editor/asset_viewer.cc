#pragma once

struct AssetViewer {
  rgg::Texture render_target;
  u32 camera_index;
};

static AssetViewer kAssetViewer;

// Just a way to verify lines work with the viewport, etc.
void EditorAssetViewerDebugLines() {
  rgg::RenderLine(
      v2f(0.f, 0.f),
      v2f(kEditorState.render_dims.x / 2.f, kEditorState.render_dims.y / 2.f), rgg::kWhite);
  rgg::RenderLine(
      v2f(0.f, 0.f),
      v2f(kEditorState.render_dims.x / 2.f, -kEditorState.render_dims.y / 2.f), rgg::kWhite);
  rgg::RenderLine(
      v2f(0.f, 0.f),
      v2f(-kEditorState.render_dims.x / 2.f, -kEditorState.render_dims.y / 2.f), rgg::kWhite);
  rgg::RenderLine(
      v2f(0.f, 0.f),
      v2f(-kEditorState.render_dims.x / 2.f, kEditorState.render_dims.y / 2.f), rgg::kWhite);

  rgg::RenderLineRectangle(EditorRenderableViewRect(), rgg::kRed);
}

void EditorAssetViewerInit() {
  static bool kInitOnce = true;
  if (!kInitOnce) {
    return;
  }
  kAssetViewer.render_target =
      rgg::CreateEmptyTexture2D(GL_RGB, kEditorState.render_dims.x, kEditorState.render_dims.y);
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
  EditorAssetViewerInit();
  rgg::CameraSwitch(kAssetViewer.camera_index);
  rgg::Camera* c = rgg::CameraGetCurrent(); 
  rgg::ModifyObserver mod(
      math::Ortho(kEditorState.render_dims.x, 0.f,
                  kEditorState.render_dims.y, 0.f, -1.f, 1.f),
      math::LookAt(c->position, c->position + v3f(0.f, 0.f, 1.f),
                   v3f(0.f, -1.f, 0.f)));
  rgg::BeginRenderTo(kAssetViewer.render_target);
  ImGuiStyle& style = ImGui::GetStyle();
  ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];
  glClear(GL_COLOR_BUFFER_BIT);
  // Fill the background with imgui's background color to maintain beauty.
  rgg::RenderRectangle(
      Rectf(-kEditorState.render_dims.x / 2.f, -kEditorState.render_dims.y / 2.f,
            kEditorState.render_dims.x, kEditorState.render_dims.y),
      v4f(imcolor.x, imcolor.y, imcolor.z, imcolor.w));
  EditorAssetViewerDebugLines();
  // Render a graph every 50 pixels
    /*for (r32 start_x = -kEditorState.render_dims.x / 2.f;
       start_x <= kEditorState.render_dims.x / 2.f;
       start_x += 50.f) {
    rgg::RenderLine(
        v2f(start_x, -kEditorState.render_dims.y / 2.f),
        v2f(start_x, kEditorState.render_dims.y / 2.f),
        v4f(1.f, 1.f, 1.f, 0.5f));
  }
  for (r32 start_y = -kEditorState.render_dims.y / 2.f;
       start_y <= kEditorState.render_dims.y / 2.f;
       start_y += 50.f) {
    rgg::RenderLine(
        v2f(-kEditorState.render_dims.x / 2.f, start_y),
        v2f(kEditorState.render_dims.x / 2.f, start_y),
        v4f(1.f, 1.f, 1.f, 0.5f));
  }*/
  rgg::EndRenderTo();
  ImGui::Begin("Debug");
  //ImGui::Text("%.2f %.2f");
  ImGui::End();
  ImGui::Image((void*)(intptr_t)kAssetViewer.render_target.reference,
               ImVec2(kEditorState.render_dims.x, kEditorState.render_dims.y));
}
