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

math::Mat4f PerspectiveProjection(v2f dims)
{
  return math::Ortho(
      dims.x, 0.f, dims.y, 0.f, 0.0f, 0.f);
}

int
main(int argc, char** argv)
{
  if (!window::Create("Graphics Explore", 800, 800)) return 1;
  
  gfx::Initialize();


  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}

    rgg::GetObserver()->projection = PerspectiveProjection(window::GetWindowSize());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderGrid();

    auto dims = window::GetWindowSize();

    imui::Text("Hello, there", dims.x / 2.f, dims.y / 2.f);
    imui::Text("Hello, there", dims.x / 2.f, dims.y / 2.f);

    imui::Render();

    imui::Reset();

    window::SwapBuffers();
  }

  return 0;
}

