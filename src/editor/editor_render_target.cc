#pragma once

void GetImGuiPanelRect(Rectf* rect) {
  ImVec2 pos = ImGui::GetWindowPos();
  ImVec2 size = ImGui::GetWindowSize();
  v2f wsize = window::GetWindowSize();
  rect->x = pos.x;
  rect->y = wsize.y - pos.y - size.y;
  rect->width = size.x;
  rect->height = size.y;
}

// TODO: This might be a decent low level renderer thing to have?
struct EditorSurface {
  bool IsValid() const { return render_target.IsValid(); }
  r32 width() const { return render_target.width(); }
  r32 height() const { return render_target.height(); }
  rgg::Camera camera;
  rgg::Surface render_target;
};

EditorSurface CreateEditorSurface(r32 width, r32 height) {
  EditorSurface surface;
  surface.camera.position = v3f(0.f, 0.f, 0.f);
  surface.camera.dir = v3f(0.f, 0.f, -1.f);
  surface.camera.up = v3f(0.f, 1.f, 0.f);
  surface.camera.viewport = v2f(width, height);
  surface.render_target = rgg::CreateSurface(GL_RGB, width, height);
  return surface;
}

void DestroyEditorSurface(EditorSurface* surface) {
  if (surface->IsValid()) {
    rgg::DestroySurface(&surface->render_target);
  }
  *surface = {};
}

class RenderToEditorSurface {
public:
  RenderToEditorSurface(const EditorSurface& surface);
  ~RenderToEditorSurface();

  rgg::ModifyObserver mod_observer_;
};

void RenderSurfaceToImGuiImage(
    const EditorSurface& surface, const rgg::Texture* texture, const Rectf& tex_rect, bool outline = false) {
  assert(surface.IsValid());
  RenderToEditorSurface render_to(surface);
  Rectf dest = Rectf(-surface.width() / 2.f, -surface.height() / 2.f, surface.width(), surface.height());
  rgg::RenderTexture(*texture, tex_rect, dest);
  ImGui::Image(
    (void*)(intptr_t)surface.render_target.texture.reference,
    ImVec2(surface.width(), surface.height()));
  if (outline) {
    rgg::RenderLineRectangle(
        Rectf(dest.x + 1.f, dest.y + 1.f, dest.width - 1.f, dest.height - 2.f), rgg::kRed);
  }
}

RenderToEditorSurface::RenderToEditorSurface(const EditorSurface& surface) : mod_observer_(surface.camera) {
  rgg::BeginRenderTo(surface.render_target);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

RenderToEditorSurface::~RenderToEditorSurface() {
  rgg::EndRenderTo();
}

class EditorRenderTarget {
public:
  void Initialize(s32 width, s32 height);
  void ReleaseSurface();

  void UpdateImguiPanelRect();
  virtual bool IsMouseInside() const;
  bool IsRenderTargetValid() const { return editor_surface_.IsValid(); }

  r32 GetRenderTargetWidth() { return editor_surface_.render_target.width(); }
  r32 GetRenderTargetHeight() { return editor_surface_.render_target.height(); }

  // Run once when render target is created. Initialization resets underlying surface.
  // So don't mess with rendering surfaces here.
  virtual void OnInitialize() {}
  // During this call the render_target is bound and all OpenGL calls are issued against that target.
  virtual void OnRender() {}
  // Do UI stuff in here so we can guarantee the render target is unbound.
  virtual void OnImGui() {}

  void ImGuiImage();
  void Render();

  rgg::Camera* camera() { return &editor_surface_.camera; }
  Rectf imgui_panel_rect() const { return imgui_panel_rect_; }

protected:
  EditorSurface editor_surface_;
  // Rectf that represents where in world space the frame lives.
  Rectf imgui_panel_rect_;
};

void EditorRenderTarget::Initialize(s32 width, s32 height) {
  if (IsRenderTargetValid()) DestroyEditorSurface(&editor_surface_);
  editor_surface_ = CreateEditorSurface(width, height);
  OnInitialize();
}

void EditorRenderTarget::ReleaseSurface() {
  DestroyEditorSurface(&editor_surface_);
}

void EditorRenderTarget::UpdateImguiPanelRect() {
  GetImGuiPanelRect(&imgui_panel_rect_);
}

bool EditorRenderTarget::IsMouseInside() const {
  return math::PointInRect(kCursor.global_screen, imgui_panel_rect_);
}


void EditorRenderTarget::ImGuiImage() {
  ImGui::Image(
      (void*)(intptr_t)editor_surface_.render_target.texture.reference,
      ImVec2(editor_surface_.render_target.width(), editor_surface_.render_target.height()));
}

void EditorRenderTarget::Render() {
  if (IsRenderTargetValid()) {
    RenderToEditorSurface render_to(editor_surface_);
    OnRender();
  }
  OnImGui();
}
