#pragma once

static b8 ImGuiImplInitForOpenGL() {
  ImGuiIO& io = ImGui::GetIO();
  return ImGui_ImplOpenGL3_Init("410");
}

static b8 ImGuiImplSetup() {
  ImGui::CreateContext(); 
  ImGui::StyleColorsDark();
  return ImGuiImplInitForOpenGL();
}

static void ImGuiImplNewFrame() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui::NewFrame();
}

static void ImGuiImplRenderDrawData() {
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
