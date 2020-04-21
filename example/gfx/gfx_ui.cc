#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "simulation/camera.cc"
#include "gfx/gfx.cc"

void
TextTest()
{
  auto dims = window::GetWindowSize();

  char buffer[64];


#if 1
  imui::PaneOptions pane_options;
  pane_options.max_height = 200.f;
  static bool show = true;
  static v2f pos(800, dims.y);
  imui::TextOptions o;
  o.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
  imui::Begin("scroll test", 0, pane_options, &pos, &show);
  imui::Text("Some text", o);
  imui::SameLine();
  static bool checked = false;
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
#endif

#if 0
  {
    imui::PaneOptions pane_options;
    pane_options.max_height = 300.f;
    static bool show = true;
    static v2f pos(800, 800);
    imui::Begin("scroll test", 0, pane_options, &pos, &show);
    imui::Text("Some text");
    imui::Text("That is going to be");
    //imui::HorizontalLine(v4f(1.f, 1.f, 1.f, 1.f));
    imui::Text("Panes");
    //imui::HorizontalLine(v4f(1.f, 1.f, 1.f, 1.f));
    static bool checked = false;
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

  static bool show_window = false;

  {
#if 0
    imui::PaneOptions pane_options;
    pane_options.height = 170.f;
    pane_options.width = 300.f;
    static bool show = true;
    static v2f pos(1200, 500);
    imui::Begin("imui test", 0, pane_options, &pos, &show);
    // imui::DockWith("scroll test");
    imui::Text("Other stuff...");
    v2f cursor = window::GetCursorPosition();
    snprintf(buffer, 64, "Mouse(%.2f,%.2f)", cursor.x, cursor.y);
    imui::Text(buffer);
    v2f delta = imui::MouseDelta();
    snprintf(buffer, 64, "Mouse Delta(%.2f,%.2f)", delta.x, delta.y);
    imui::Text(buffer);
    snprintf(buffer, 64, "Mouse Down(%i)", imui::IsMouseDown());
    imui::Text(buffer);
    snprintf(buffer, 64, "IMUI Panes(%i)", imui::kUsedPane);
    imui::Text(buffer);
    if (imui::Button(32.f, 32.f, v4f(1.f, 0.f, 0.f, 1.f)).clicked) {
      show_window = !show_window;
    }
    imui::End();
#endif
  }

  {
    if (show_window) {
      imui::PaneOptions pane_options;
      static bool show = true;
      static v2f pos(1500, 300);
      imui::Begin("dynamically created pane", 0, pane_options, &pos, &show);
      imui::Text("Test..");
      imui::Text("Test 2");
      imui::Text("Test 3");
      imui::End();
    }
  }

#endif
  {
    static v2f start(0.f, 400.f);
    static bool show = true;
    imui::DebugPane("DEBUG TAG 0", 0, &start, &show);
  }

}

int
main(int argc, char** argv)
{
  window::CreateInfo create_info;
  create_info.window_width = 1920;
  create_info.window_height = 1080;
  gfx::Initialize(create_info);
  v2f dims = window::GetWindowSize();

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch (event.type) {
        case MOUSE_DOWN: {
          if (event.button == BUTTON_LEFT) {
            imui::MouseDown(event.position, event.button, 0);
          }
        } break;
        case MOUSE_UP: {
          if (event.button == BUTTON_LEFT) {
            imui::MouseUp(event.position, event.button, 0);
          }
        } break;
        case KEY_DOWN: {
          switch (event.key) {
            case 27 /* ESC */: {
              exit(1);
            } break;
            default: break;
          }
        } break;
        case MOUSE_WHEEL: {
          imui::MouseWheel(event.wheel_delta, 0);
        } break;
        default: break;
      }
    }

    const v2f cursor = window::GetCursorPosition();
    static int i = 0;
    if (imui::MousePosition(cursor, 0)) {
      // printf("mouse in bounds %i\n", i++);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.0f, 0.0f, 1.0f);

    TextTest();
    rgg::ModifyObserver mod(math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f),
                            math::Identity());
    rgg::DebugRenderPrimitives();
    imui::Render(0);
    imui::ResetAll();
    window::SwapBuffers();
    rgg::DebugReset();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

