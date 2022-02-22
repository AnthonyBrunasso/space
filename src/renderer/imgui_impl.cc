#pragma once

static b8 ImGuiImplInitForOpenGL() {
  GLint major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  ImGuiIO& io = ImGui::GetIO();
  v2f window_size = window::GetWindowSize();
  io.DisplaySize.x = window_size.x;
  io.DisplaySize.y = window_size.y;
  return ImGui_ImplOpenGL3_Init("#version 130");
}

static b8 ImGuiImplSetup() {
  ImGui::CreateContext(); 
  ImGui::StyleColorsDark();
  return ImGuiImplInitForOpenGL();
}

static void ImGuiImplNewFrame() {
  ImGui_ImplOpenGL3_NewFrame();
}

static void ImGuiImplRenderDrawData() {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static bool ImGuiImplProcessEvent(const PlatformEvent& event) {
  ImGuiIO& io = ImGui::GetIO();
  switch (event.type) {
    case MOUSE_MOVE: {
      v2f dims = window::GetWindowSize();
      io.AddMousePosEvent(event.position.x, dims.y - event.position.y);
      return true;
    } break;
    case MOUSE_WHEEL: {
      // TODO: Wheel event for x and y????
      io.AddMouseWheelEvent(0.f, event.wheel_delta > 0.f ? 1.f : -1.f);
      return true;
    } break;
    case MOUSE_DOWN:
    case MOUSE_UP: {
      int mouse_button = -1;
      if (event.button == BUTTON_LEFT) mouse_button = 0;
      if (event.button == BUTTON_RIGHT) mouse_button = 1;
      if (event.button == BUTTON_MIDDLE) mouse_button = 2;
      if (mouse_button == -1) break;
      io.AddMouseButtonEvent(mouse_button, event.type == MOUSE_DOWN);
      return true;
    } break;
    case KEY_DOWN:
    case KEY_UP: {
      // TODO:
      return true;
    } break;
  }
  return false;
}
