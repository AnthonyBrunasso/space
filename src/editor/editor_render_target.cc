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

void GetImGuiLastItemRect(Rectf* rect) {
  ImVec2 pos = ImGui::GetItemRectMin();
  ImVec2 size = ImGui::GetItemRectSize();
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
  v2f Dims() const { return v2f( width(), height() ); }
  rgg::Camera camera;
  rgg::Surface render_target;
};

EditorSurface CreateEditorSurface(r32 width, r32 height) {
  EditorSurface surface;
  surface.camera.position = v3f(0.f, 0.f, 0.f);
  surface.camera.dir = v3f(0.f, 0.f, -1.f);
  surface.camera.up = v3f(0.f, 1.f, 0.f);
  surface.camera.viewport = v2f(width, height);
  surface.render_target = rgg::CreateSurface(GL_RGB, (u64)width, (u64)height);
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
  void UpdateCursor();

  void UpdateImguiPanelRect();
  virtual bool IsMouseInside() const;
  bool IsMouseInsideEditorSurface() const;
  bool IsRenderTargetValid() const { return editor_surface_.IsValid(); }

  r32 GetRenderTargetWidth() { return editor_surface_.render_target.width(); }
  r32 GetRenderTargetHeight() { return editor_surface_.render_target.height(); }
  v2f GetRenderTargetDims() { return v2f(GetRenderTargetWidth(), GetRenderTargetHeight()); }

  // Get the camera rect for the surface - this is the min / max points of render surface in world space
  // considering the position of the camera.
  Rectf GetCameraRect();
  // Then scale it if we care about rendering stuff.
  Rectf GetCameraRectScaled();

  // Run once when render target is created. Initialization resets underlying surface.
  // So don't mess with rendering surfaces here.
  virtual void OnInitialize() {}
  // During this call the render_target is bound and all OpenGL calls are issued against that target.
  virtual void OnRender() {}
  // Do UI stuff in here so we can guarantee the render target is unbound.
  virtual void OnImGui() {}
  // Will be dispatched to the active EditorRenderTarget.
  virtual void OnFileSelected(const std::string& filename) {}

  void ImGuiImage();
  void RenderGrid(v4f color, bool alternate_alpha = false);
  void RenderCursorAsRect();
  void RenderAxis();
  void Render();
  
  rgg::Camera* camera() { return &editor_surface_.camera; }
  Rectf imgui_panel_rect() const { return imgui_panel_rect_; }
  EditorGrid* grid() { return &grid_; }

  const EditorCursor& cursor() const { return cursor_; }

  r32 scale_ = 1.0f;

protected:
  EditorSurface editor_surface_;
  // Rectf that represents where in world space the frame lives.
  Rectf imgui_panel_rect_;
  // Rect for where the actual render surface is.
  Rectf imgui_editor_surface_rect_;
  // Grid used for rendering / cursor shenanigans.
  EditorGrid grid_;
  // Cursor as relative to the surface in editor_surface
  EditorCursor cursor_;
};

void EditorRenderTarget::Initialize(s32 width, s32 height) {
  if (IsRenderTargetValid()) DestroyEditorSurface(&editor_surface_);
  editor_surface_ = CreateEditorSurface((r32)width, (r32)height);
  OnInitialize();
}

void EditorRenderTarget::ReleaseSurface() {
  DestroyEditorSurface(&editor_surface_);
}

void EditorRenderTarget::UpdateCursor() {
  v2f cursor = window::GetCursorPosition();
  cursor_.global_screen = cursor;
  ImGuiStyle& style = ImGui::GetStyle();
  //LOG(INFO, "%.2f - %.2f", cursor.x, imgui_panel_rect_.x);
  cursor_.local_screen = v2f(
      cursor.x - imgui_editor_surface_rect_.x, cursor.y - imgui_editor_surface_rect_.y);
  // User sees a scaled version of the world therefore a cursor placed in that space is in scaled world
  // space
  cursor_.world_scaled = cursor_.local_screen - (editor_surface_.Dims() / 2.f);
  // To get the inverse world space we divide out the scale.
  cursor_.world = Roundf(cursor_.world_scaled / scale_);
  rgg::Camera* camera = &editor_surface_.camera;
  if (camera) {
    cursor_.world += (camera->position.xy() / scale_);
    cursor_.world_scaled += camera->position.xy();
  }
  cursor_.is_in_viewport = math::PointInRect(cursor_.global_screen, imgui_editor_surface_rect_);
  // Move the cursor into grid space in the world
  v2f cursor_relative = cursor_.world - grid_.GetOrigin();
  // Clamp to nearest grid edge.
  Rectf rgrid;
  rgrid.x = roundf(cursor_relative.x - ((int)roundf(cursor_relative.x) % grid_.cell_width));
  rgrid.y = roundf(cursor_relative.y - ((int)roundf(cursor_relative.y) % grid_.cell_height));
  rgrid.x = cursor_relative.x < 0.f ? rgrid.x - grid_.cell_width : rgrid.x;
  rgrid.y = cursor_relative.y < 0.f ? rgrid.y - grid_.cell_height : rgrid.y;
  rgrid.width = (r32)grid_.cell_width;
  rgrid.height = (r32)grid_.cell_height;
  cursor_.world_grid_cell = rgrid;
  cursor_.world_grid_cell.x += grid_.GetOrigin().x;
  cursor_.world_grid_cell.y += grid_.GetOrigin().y;
  cursor_.world_clamped = Roundf(rgrid.NearestEdge(cursor_relative)) + grid_.GetOrigin();
}

void EditorRenderTarget::UpdateImguiPanelRect() {
  GetImGuiPanelRect(&imgui_panel_rect_);
}

bool EditorRenderTarget::IsMouseInside() const {
  return math::PointInRect(cursor_.global_screen, imgui_panel_rect_);
}

bool EditorRenderTarget::IsMouseInsideEditorSurface() const {
  return math::PointInRect(cursor_.global_screen, imgui_editor_surface_rect_);
}

Rectf EditorRenderTarget::GetCameraRect() {
  Rectf r = GetCameraRectScaled();
  r.x /= scale_;
  r.y /= scale_;
  r.width /= scale_;
  r.height /= scale_;
  return r;
}

// Camera position is already scaled.
Rectf EditorRenderTarget::GetCameraRectScaled() {
  v2f dims = editor_surface_.Dims();
  Rectf r(v2f(-dims.x / 2.f, -dims.y / 2.f), editor_surface_.Dims());
  if (!camera()) return r;
  r.x += camera()->position.x;
  r.y += camera()->position.y;
  return r;
}

void EditorRenderTarget::ImGuiImage() {
  ImGui::Image(
      (void*)(intptr_t)editor_surface_.render_target.texture.reference,
      ImVec2(editor_surface_.render_target.width(), editor_surface_.render_target.height()));
  GetImGuiLastItemRect(&imgui_editor_surface_rect_);
}

r32 __get_grid_line_color(s32 alpha_num, s32 alpha_1, s32 alpha_2, s32 alpha_3) {
  if (alpha_num == alpha_1) return .1f;
  if (alpha_num == alpha_2) return .2f;
  if (alpha_num == alpha_3) return .4f;
  return .1f;
}

void EditorRenderTarget::RenderGrid(v4f color, bool alternate_alpha) {
  // Everything must be scaled to respect our zoom.
  v2f start_scaled = grid_.GetOrigin() * scale_;
  const Rectf& view_rect_scaled = GetCameraRectScaled();
  s32 scaled_width = grid_.cell_width * scale_;
  s32 scaled_height = grid_.cell_height * scale_;
  assert(scaled_width != 0 && scaled_height != 0);
  s32 alpha_1_width = scaled_width;
  s32 alpha_2_width = alpha_1_width * 2;
  s32 alpha_3_width = alpha_2_width * 2;
  s32 alpha_1_height = scaled_height;
  s32 alpha_2_height = alpha_1_height * 2;
  s32 alpha_3_height = alpha_2_height * 2;
  // Draw lines right
  s32 alpha_num = 0;
  for (r32 start_x = start_scaled.x; start_x <= view_rect_scaled.Max().x; start_x += scaled_width) {
    if (alternate_alpha)
      color.w = __get_grid_line_color(alpha_num, alpha_1_width, alpha_2_width, alpha_3_width);
    rgg::RenderLine(v2f(start_x, view_rect_scaled.Min().y), v2f(start_x, view_rect_scaled.Max().y), color);
    alpha_num += scaled_width;
    if (alpha_num > alpha_3_width) alpha_num = alpha_1_width;
  }
  // Draw lines left
  alpha_num = alpha_1_width;
  for (r32 start_x = start_scaled.x - scaled_width; start_x >= view_rect_scaled.Min().x; start_x -= scaled_width) {
    if (alternate_alpha)
      color.w = __get_grid_line_color(alpha_num, alpha_1_width, alpha_2_width, alpha_3_width);
    rgg::RenderLine(v2f(start_x, view_rect_scaled.Min().y), v2f(start_x, view_rect_scaled.Max().y), color);
    alpha_num += scaled_width;
    if (alpha_num > alpha_3_width) alpha_num = alpha_1_width;
  }
  // Draw lines up
  alpha_num = 0;
  for (r32 start_y = start_scaled.y; start_y <= view_rect_scaled.Max().y; start_y += scaled_height) {
    if (alternate_alpha)
      color.w = __get_grid_line_color(alpha_num, alpha_1_height, alpha_2_height, alpha_3_height);
    rgg::RenderLine(v2f(view_rect_scaled.Min().x, start_y), v2f(view_rect_scaled.Max().x, start_y), color);
    alpha_num += scaled_height;
    if (alpha_num > alpha_3_height) alpha_num = alpha_1_height;
  }
  // Draw lines down
  alpha_num = alpha_1_height;
  for (r32 start_y = start_scaled.y - scaled_height; start_y >= view_rect_scaled.Min().y; start_y -= scaled_height) {
    if (alternate_alpha)
      color.w = __get_grid_line_color(alpha_num, alpha_1_height, alpha_2_height, alpha_3_height);
    rgg::RenderLine(v2f(view_rect_scaled.Min().x, start_y), v2f(view_rect_scaled.Max().x, start_y), color);
    alpha_num += scaled_height;
    if (alpha_num > alpha_3_height) alpha_num = alpha_1_height;
  }
}

void EditorRenderTarget::RenderCursorAsRect() {
  Rectf scaled_rect = cursor_.world_grid_cell;
  scaled_rect.x *= scale_;
  scaled_rect.y *= scale_;
  scaled_rect.width *= scale_;
  scaled_rect.height *= scale_;
  rgg::RenderLineRectangle(scaled_rect, rgg::kRed);
}

void EditorRenderTarget::RenderAxis() {
  v2f origin_scaled = grid_.GetOrigin() * scale_;
  const Rectf& view_rect_scaled = GetCameraRectScaled();
  rgg::RenderLine(v2f(origin_scaled.x, view_rect_scaled.Min().y),
                  v2f(origin_scaled.x, view_rect_scaled.Max().y),
                  v4f(0.f, 1.f, 0.f, 0.5f));
  rgg::RenderLine(v2f(view_rect_scaled.Min().x, origin_scaled.y),
                  v2f(view_rect_scaled.Max().x, origin_scaled.y),
                  v4f(0.f, 0.f, 1.f, 0.5f));
}

void EditorRenderTarget::Render() {
  if (IsRenderTargetValid()) {
    RenderToEditorSurface render_to(editor_surface_);
    OnRender();
  }
  OnImGui();
}
