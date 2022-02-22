#include "common/include.cc"

struct EditorState {
  u64 frame_rate = 60;
  window::CreateInfo window_create_info;
  platform::Clock clock;
};

static EditorState kEditorState;

void EditorExit() {
  exit(0);
}

void EditorProcessEvent(PlatformEvent& event) {
  switch(event.type) {
    case KEY_DOWN: {
      switch (event.key) {
        case KEY_ESC: {
          EditorExit();
        } break;
      }
    } break;
  }
}

void EditorMainMenuFile() {
  if (ImGui::MenuItem("New")) {
  }
  if (ImGui::MenuItem("Open", "Ctrl+O")) {
  }
}

void EditorMainMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      EditorMainMenuFile();
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

s32 main(s32 argc, char** argv) {
  kEditorState.window_create_info.window_width = 1920;
  kEditorState.window_create_info.window_height = 1080;
  if (!window::Create("Space", kEditorState.window_create_info)) {
    return 1;
  }

  if (!rgg::Initialize()) {
    return 1;
  }

  bool show_demo_window = true;

  while (1) {
    platform::ClockStart(&kEditorState.clock);
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

    EditorMainMenuBar();

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
