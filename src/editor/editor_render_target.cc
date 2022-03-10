#pragma once

class EditorRenderTarget {
public:
  void Initialize(s32 width, s32 height);
  void SetupCamera(s32 width, s32 height);
  void SetupTexture(s32 width, s32 height);

  void UpdateImguiPanelRect();
  bool IsMouseInside() const;

  // Do imgui stuff + rendering to texture.
  virtual void OnRender() {}
  void ImGuiImage();
  void Render();

  rgg::Camera* camera() { return &camera_; }

private:
  // Camera to render from perspective
  rgg::Camera camera_;
  // The render target all draw calls will go to.
  rgg::Texture render_target_;
  // Rectf that represents where in world space the frame lives.
  Rectf imgui_panel_rect_;
};

void EditorRenderTarget::Initialize(s32 width, s32 height) {
  SetupCamera(width, height);
  SetupTexture(width, height);
}

void EditorRenderTarget::SetupCamera(s32 width, s32 height) {
  // Not sure these ever have to change for UI cameras.
  camera_.position = v3f(0.f, 0.f, 0.f);
  camera_.dir = v3f(0.f, 0.f, -1.f);
  camera_.up = v3f(0.f, 1.f, 0.f);
  camera_.viewport = v2f(width, height);
}

void EditorRenderTarget::SetupTexture(s32 width, s32 height) {
  if (render_target_.IsValid()) {
    rgg::DestroyTexture2D(&render_target_);
  }
  render_target_ = rgg::CreateEmptyTexture2D(GL_RGB, width, height);
}

void EditorRenderTarget::UpdateImguiPanelRect() {
  ImVec2 pos = ImGui::GetWindowPos();
  ImVec2 size = ImGui::GetWindowSize();
  v2f wsize = window::GetWindowSize();
  imgui_panel_rect_.x = pos.x;
  imgui_panel_rect_.y = wsize.y - pos.y - size.y;
  imgui_panel_rect_.width = size.x;
  imgui_panel_rect_.height = size.y;
}

bool EditorRenderTarget::IsMouseInside() const {
  return math::PointInRect(kCursor.global_screen, imgui_panel_rect_);
}

void EditorRenderTarget::ImGuiImage() {
  ImGui::Image(
      (void*)(intptr_t)render_target_.reference,
      ImVec2(render_target_.width, render_target_.height));
}

void EditorRenderTarget::Render() {
  rgg::ModifyObserver mod(camera_);
  rgg::BeginRenderTo(render_target_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  OnRender();
  rgg::EndRenderTo();
}
