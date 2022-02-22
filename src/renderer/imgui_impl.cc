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
