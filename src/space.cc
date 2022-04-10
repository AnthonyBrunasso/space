#include "common/include.cc"
#include "editor/editor.cc"

bool SetupWorkingDirectory() {
  bool result = false;
  // Check the directory the binary is run from and one backwards then give up.
  if (!filesystem::WorkingDirectoryContains("asset")) {
#ifdef _WIN32
    filesystem::ChangeDirectory("..\\");
#else
    filesystem::ChangeDirectory("../");
#endif
    result = filesystem::WorkingDirectoryContains("asset"); 
  } else {
    result = true;
  }
  return result;
}

s32 main(s32 argc, char** argv) {
  // Cuz my windows machine gots a bigger montior
#ifdef _WIN32
  kEditor.window_create_info.window_width = 1920;
  kEditor.window_create_info.window_height = 1080;
#else
  kEditor.window_create_info.window_width = 1600;
  kEditor.window_create_info.window_height = 900;
#endif

  if (!SetupWorkingDirectory()) {
    LOG(ERR, "Unable to setup working directory.");
    return 1;
  }

  LOG(INFO, "Working dir: %s", filesystem::GetWorkingDirectory());

  if (!window::Create("Space", kEditor.window_create_info)) {
    return 1;
  }

  if (!rgg::Initialize()) {
    return 1;
  }

  bool show_demo_window = false;

  while (1) {
    platform::ClockStart(&kEditor.clock);
    if (window::ShouldClose()) break;
    ImGuiImplNewFrame();
    ImGui::NewFrame();

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      ImGuiImplProcessEvent(event);
      EditorProcessEvent(event);
    }

    // TODO: Abstract into renderer calls.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.1f, .1f, .13f, 1.f);

    EditorMain();
    
    if (show_demo_window) {
      ImGui::ShowDemoWindow(&show_demo_window);
    }


    rgg::DebugRenderUIPrimitives();

    ImGui::Render();
    ImGuiImplRenderDrawData();
    ImGui::EndFrame();

    window::SwapBuffers();
  }

  return 0;
}
