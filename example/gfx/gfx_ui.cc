#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "simulation/camera.cc"
#include "gfx/gfx.cc"

void
AlignToGrid(v2f grid, math::Rectf* world)
{
  float x_align = fmodf(world->min.x, grid.x);
  float y_align = fmodf(world->min.y, grid.y);

  world->min.x -= x_align + grid.x;
  world->min.y -= y_align + grid.y;
}

void
RenderGrid()
{
  auto dims = window::GetWindowSize();
  rgg::RenderLine(v3f(-dims.x / 2.f, 0.0f, 0.0f),
                  v3f(dims.x / 2.f, 0.0f, 0.0f),
                  v4f(1.0f, 0.0f, 0.0f, 1.0f));
  rgg::RenderLine(v3f(0.0f, -dims.y / 2.f, 0.0f),
                  v3f(0.0f, dims.y / 2.f, 0.0f),
                  v4f(0.0f, 0.0f, 1.0f, 1.0f));
  math::Rectf world;
  world.min = {-dims.x / 2.f, -dims.y / 2.f};
  world.max = {dims.x / 2.f, dims.y / 2.f};

  const v2f grid2(50.f, 50.f);
  math::Rectf world2 = world;
  AlignToGrid(grid2, &world2);
  rgg::RenderGrid(grid2, world2, v4f(0.207f, 0.317f, 0.360f, 0.60f));

  const v2f grid1(25.f, 25.f);
  math::Rectf world1 = world;
  AlignToGrid(grid1, &world1);
  rgg::RenderGrid(grid1, world1, v4f(0.050f, 0.215f, 0.050f, 0.55f));
}

math::Mat4f Projection(v2f dims)
{
  return math::Ortho(dims.x, 0.f, dims.y, 0.f, 0.0f, 0.f);
}

void
RenderStringWithBoundingBox(const char* msg, v2f pos, v2f dims)
{
  int msg_len = strlen(msg);
  imui::TextOptions options;
  options.color = v4f(0.35f, 0.12f, 0.70f, 1.f);
  imui::Text(msg, pos, options);
  rgg::ModifyObserver mod(
      math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f), math::Identity());
  rgg::RenderLineRectangle(rgg::GetTextRect(msg, msg_len, pos),
                           v4f(1.0f, 0.0f, 0.0f, 1.0f));
}

// Test used for testing bounding box on sentences and using it to do modal
// text rendering stuff. Probably test that this looks decent if switching
// font sheets or messing with text code.
void
TextTest()
{
  auto dims = window::GetWindowSize();
#if 0
  RenderStringWithBoundingBox(
      "quick", v2f(dims.x / 2.f, dims.y / 2.f), dims);
  RenderStringWithBoundingBox(
      "The quick brown fox", v2f(dims.x / 2.f, dims.y / 2.f + 40.0f), dims);
  RenderStringWithBoundingBox(
      "this, sentence, has, commas", v2f(dims.x / 2.f, dims.y / 2.f + 80.0f), dims);
  RenderStringWithBoundingBox(
      "I kInd. OF, h4te_ font", v2f(dims.x / 2.f, dims.y / 2.f + 120.0f), dims);
  RenderStringWithBoundingBox(
      "1.32241 + 32569 = NUMBER", v2f(dims.x / 2.f, dims.y / 2.f + 160.0f), dims);
#endif


  {
    imui::PaneOptions pane_options;
    imui::Begin(v2f(dims.x / 2.f - 400.0f, dims.y / 2.f + 160.0f), pane_options);
    imui::TextOptions text_options;
    text_options.highlight_color = v4f(1.0f, 0.0f, 0.0f, 1.0f);
    text_options.color = gfx::kWhite;
    static bool is_clicked = false;
    if (imui::Text("Debug", text_options).highlighted) {
      imui::Indent(5);
      imui::Text("1.32241 + 32569 = NUMBER");
      imui::Text("I kInd. OF, h4te_ font");
      imui::Text("The quick brown fox");
      imui::Text("quick");
      imui::Text("To TL td tj Tj Pj pj PJ");
      imui::Text("1023");
      imui::Indent(-5);
    }
    imui::Text("Test");
    imui::End();
  }

  {
    imui::PaneOptions pane_options;
    imui::TextOptions text_options;
    text_options.scale = 0.75f;
    imui::Begin(v2f(dims.x / 2.f + 200.0f, dims.y / 2.f + 160.0f), pane_options);
    imui::Text("This font is smaller.", text_options);
    imui::Text("Than the normal font.", text_options);
    imui::Text("2341 + 3234 = 18923", text_options);
    imui::End();
  }
}

int
main(int argc, char** argv)
{
  gfx::Initialize();

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {
      switch (event.type) {
        case MOUSE_DOWN: {
          if (event.button == BUTTON_LEFT) {
            imui::MouseClick(event.position);
          }
        } break;
        default: break;
      }
    }

    auto dims = window::GetWindowSize();

    rgg::GetObserver()->projection = Projection(dims);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderGrid();
    TextTest();
    imui::Render();
    imui::Reset();
    window::SwapBuffers();
    platform::sleep_ms(10);
  }

  return 0;
}

