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

  {
    imui::PaneOptions pane_options;
    pane_options.title = "Button Test";
    static bool show = true;
    static v2f pos(1000, 300);
    imui::Begin(&pos, 0, pane_options, &show);
    static int i = 0;
    if (imui::ButtonCircle(15.f, v4f(1.f, 0.f, 0.f, 1.f)).clicked) {
      ++i;
    }
    snprintf(buffer, 64, "Button Click: %i", i);
    imui::Text(buffer);
    imui::Text("Some text");
    imui::Text("That is going to be");
    imui::HorizontalLine(v4f(1.f, 1.f, 1.f, 1.f));
    imui::Text("Seperated");
    imui::Text("By a line!");
    imui::ToggleSameLine();
    imui::Text("This ");
    imui::Text("is ");
    imui::Text("the ");
    imui::Text("same ");
    imui::Text("line ");
    imui::Button(25, 25, v4f(1.f, 0.5f, 0.3f, 1.f));
    imui::ToggleNewLine();
    imui::Text("Next line...");
    imui::End();
  }

  {
    imui::PaneOptions pane_options;
    pane_options.title = "Mouse Test";
    static bool show = true;
    static v2f pos(1200, 500);
    imui::Begin(&pos, 0, pane_options, &show);
    imui::Text("Other stuff...");
    v2f cursor = window::GetCursorPosition();
    snprintf(buffer, 64, "Mouse(%.2f,%.2f)", cursor.x, cursor.y);
    imui::Text(buffer);
    v2f delta = imui::MouseDelta();
    snprintf(buffer, 64, "Mouse Delta(%.2f,%.2f)", delta.x, delta.y);
    imui::Text(buffer);
    snprintf(buffer, 64, "Mouse Down(%i)", imui::IsMouseDown());
    imui::Text(buffer);
    imui::End();
  }
}

int
main(int argc, char** argv)
{
  window::CreateInfo create_info;
  create_info.window_width = 1920;
  create_info.window_height = 1080;
  gfx::Initialize(create_info);


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
    imui::Render(0);
    imui::ResetAll();
    window::SwapBuffers();
    platform::sleep_usec(10*1000);
  }

  return 0;
}

