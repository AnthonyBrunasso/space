#pragma once

class EditorRenderTarget {
public:
  void Initialize(s32 width, s32 height);
  void SetupCamera(s32 width, s32 height);
  void SetupTexture(s32 width, s32 height);
  void ReleaseSurface();

  void UpdateImguiPanelRect();
  bool IsMouseInside() const;
  bool IsRenderTargetValid() const { return render_target_.IsValid(); }

  // Run once when render target is created. Initialization resets underlying surface.
  // So don't mess with rendering surfaces here.
  virtual void OnInitialize() {}
  // During this call the render_target is bound and all OpenGL calls are issued against that target.
  virtual void OnRender() {}
  // Do UI stuff in here so we can guarantee the render target is unbound.
  virtual void OnImGui() {}

  void ImGuiImage();
  void Render();

  rgg::Camera* camera() { return &camera_; }
  Rectf imgui_panel_rect() const { return imgui_panel_rect_; }

private:
  // Camera to render from perspective
  rgg::Camera camera_;
  // The render target all draw calls will go to.
  rgg::Surface render_target_;
  // Rectf that represents where in world space the frame lives.
  Rectf imgui_panel_rect_;
};

void EditorRenderTarget::Initialize(s32 width, s32 height) {
  SetupCamera(width, height);
  SetupTexture(width, height);
  OnInitialize();
}

void EditorRenderTarget::SetupCamera(s32 width, s32 height) {
  // Not sure these ever have to change for UI cameras.
  camera_.position = v3f(0.f, 0.f, 0.f);
  camera_.dir = v3f(0.f, 0.f, -1.f);
  camera_.up = v3f(0.f, 1.f, 0.f);
  camera_.viewport = v2f(width, height);
}

void EditorRenderTarget::ReleaseSurface() {
  if (render_target_.IsValid()) {
    rgg::DestroySurface(&render_target_);
  }
}

void EditorRenderTarget::SetupTexture(s32 width, s32 height) {
  ReleaseSurface();
  render_target_ = rgg::CreateSurface(GL_RGB, width, height);
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
      (void*)(intptr_t)render_target_.texture.reference,
      ImVec2(render_target_.texture.width, render_target_.texture.height));
}

void EditorRenderTarget::Render() {
  rgg::ModifyObserver mod(camera_);
  rgg::BeginRenderTo(render_target_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  OnRender();
  rgg::EndRenderTo();
  OnImGui();
}
