#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "renderer/camera.cc"
#include "renderer/imui.cc"
#include "memory/memory.cc"

void
Render()
{
  auto dims = window::GetWindowSize();

  char buffer[64];

  imui::PaneOptions pane_options;
  static b8 show = true;
  static v2f pos(300, 300);
  imui::Begin("Small", imui::kEveryoneTag, pane_options, &pos, &show);
  imui::Text("No indent");
  imui::Indent(2);
  imui::Text("Indent(1)");
  imui::Text("Indent(1)");
  imui::Indent(-2);
  imui::Text("No indent");
  imui::Indent(2);
  imui::Text("Indent(1)");
  imui::Text("Indent(1)");
  imui::Indent(-2);
  imui::End();
#if 0
  imui::PaneOptions pane_options;
  pane_options.max_height = 200.f;
  static b8 show = true;
  static v2f pos(300, 300);
  imui::TextOptions o;
  o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  imui::Begin("Small", imui::kEveryoneTag, pane_options, &pos, &show);
  imui::Text("Some text", o);
  imui::SameLine();
  static b8 checked = false;
  imui::Checkbox(16.f, 16.f, &checked);
  imui::Text(" Sguff...", o);
  imui::NewLine();
  imui::SameLine();
  imui::Button(35.f, 35.f, v4f(1.f, 0.f, 0.f, .5f));
  imui::Text("test..");
  imui::NewLine();
  imui::SameLine();
  imui::Button(35.f, 35.f, v4f(1.f, 0.f, 0.f, .5f));
  imui::Text("test 2..");
  imui::End();

  {
    imui::PaneOptions pane_options;
    pane_options.max_height = 300.f;
    static b8 show = true;
    static v2f pos(800, 800);
    imui::Begin("Big", imui::kEveryoneTag, pane_options, &pos, &show);
    imui::Text("Some text");
    imui::Text("That is going to be");
    //imui::HorizontalLine(v4f(1.f, 1.f, 1.f, 1.f));
    imui::Text("Panes");
    //imui::HorizontalLine(v4f(1.f, 1.f, 1.f, 1.f));
    static b8 checked = false;
    imui::SameLine();
    imui::TextOptions o;
    o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
    imui::Text("Test", o);
    imui::Checkbox(16.f, 16.f, &checked);
    imui::NewLine();
    imui::SameLine();
    imui::Checkbox(16.f, 16.f, &checked);
    imui::Text("Test", o);
    imui::NewLine();
    imui::Text("Seperated");
    imui::Text("By a line!");
    imui::SameLine();
    imui::Text("This is the same line ");
    imui::Button(26, 26, v4f(1.f, 0.5f, 0.3f, 1.f));
    imui::NewLine();
    imui::Text("Next line...");
    imui::SameLine();
    if (imui::ButtonCircle(15.f, v4f(1.f, 0.f, 0.f, 1.f)).clicked) {
      imui::VerticalScrollDelta(-1.f);
    }
    if (imui::ButtonCircle(15.f, v4f(0.f, 0.f, 1.f, 1.f)).clicked) {
      imui::VerticalScrollDelta(1.f);
    }
    imui::NewLine();
    imui::Text("Stuff.");
    imui::Text("Stuff.");
    imui::Text("Stuff.");
    imui::Text("More Stuff.");
    imui::Text("MoreMore  Stuff.");
    imui::Text("MoreMoreMoreMoreMore     Stuff.");
    imui::Text("MoreMore  Stuff.");
    imui::Text("The");
    imui::Text("Brown");
    imui::Text("Fox");
    imui::Text("Jumps");
    imui::Text("Over");
    imui::Text("The");
    imui::Text("Lazy");
    imui::Text("Fox");
    imui::End();
  }

  {
    imui::PaneOptions pane_options;
    pane_options.height = 170.f;
    pane_options.width = 300.f;
    static b8 show = true;
    static v2f pos(1200, 500);
    imui::Begin("imui test", imui::kEveryoneTag, pane_options, &pos, &show);
    imui::Text("Test 1");
    imui::Text("Test 2");
    imui::Text("Test 3");
    imui::End();
  }

  {
    static v2f pos(500, 500);
    static b8 show = true;
    imui::DebugPane("Debug UI", imui::kEveryoneTag, &pos, &show);
  }
#endif
  rgg::DebugRenderPrimitives();
  imui::Render(imui::kEveryoneTag);
  window::SwapBuffers();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int
main(int argc, char** argv)
{
  memory::Initialize(MiB(64));
  window::CreateInfo create_info;
  create_info.window_width = 1920;
  create_info.window_height = 1080;
  window::Create("UI Test", create_info);

  if (!rgg::Initialize()) {
    return 1;
  }

  v2f dims = window::GetWindowSize();

  rgg::Camera camera;
  camera.position = v3f(0.f, 1.f, 100.f);
  camera.dir = v3f(0.f, 0.f, -1.f);
  camera.up = v3f(0.f, 1.f, 0.f);
  camera.mode = rgg::kCameraBrowser;
  camera.speed = v3f(5.f, 5.f, 5.f);
  camera.viewport = dims;
  rgg::CameraInit(camera);


  rgg::GetObserver()->projection = rgg::DefaultPerspective(dims);
  platform::Clock game_clock;

  u64 frame_target_usec = 1000.f * 1000.f / 60;

  while (!window::ShouldClose()) {
    platform::ClockStart(&game_clock);
    imui::ResetTag(imui::kEveryoneTag);
    rgg::DebugReset();

    imui::MousePosition(window::GetCursorPosition(), imui::kEveryoneTag);

    PlatformEvent event;
    while (window::PollEvent(&event)) {
      rgg::CameraUpdateEvent(event);
      switch(event.type) {
        case KEY_DOWN: {
          switch (event.key) {
            case 27 /* ESC */: {
              exit(1);
            } break;
          }
        } break;
        case MOUSE_DOWN: {
          imui::MouseDown(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_UP: {
          imui::MouseUp(event.position, event.button, imui::kEveryoneTag);
        } break;
        case MOUSE_WHEEL: {
          imui::MouseWheel(event.wheel_delta, imui::kEveryoneTag);
        } break;
        default: break;
      }
    }

    Render();

    const u64 elapsed_usec = platform::ClockEnd(&game_clock);
    if (frame_target_usec > elapsed_usec) {
      u64 wait_usec = frame_target_usec - elapsed_usec;
      platform::Clock wait_clock;
      platform::ClockStart(&wait_clock);
      while (platform::ClockEnd(&wait_clock) < wait_usec) {}
    }
  }

  return 0;
}

