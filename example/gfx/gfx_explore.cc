
#include "platform/platform.cc"
#include "gl/gl.cc"
#include "renderer/renderer.cc"
#include "simulation/camera.cc"

void
AlignToGrid(math::Vec2f grid, math::Rectf* world)
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
  math::Rectf world;
  world.min = {-dims.x / 2.f, -dims.y / 2.f};
  world.max = {dims.x / 2.f, dims.y / 2.f};

  const math::Vec2f grid2(50.f, 50.f);
  math::Rectf world2 = world;
  AlignToGrid(grid2, &world2);
  rgg::RenderGrid(grid2, world2, math::Vec4f(0.207f, 0.317f, 0.360f, 0.60f));

  const math::Vec2f grid1(25.f, 25.f);
  math::Rectf world1 = world;
  AlignToGrid(grid1, &world1);
  rgg::RenderGrid(grid1, world1, math::Vec4f(0.050f, 0.215f, 0.050f, 0.55f));
}

math::Mat4f PerspectiveProjection()
{
  auto sz = window::GetWindowSize();
  //return math::CreatePerspectiveMatrix<float>(sz.x, sz.y, .1f, 100.f, 67.f);
  return math::CreateOrthographicMatrix<float>(
      sz.x, 0.f, sz.y, 0.f, 0.0f, 0.f);
}

int
main(int argc, char** argv)
{
  if (!window::Create("Graphics Explore", 800, 800)) return 1;
  
  rgg::Initialize();

  rgg::Texture texture = rgg::CreateTexture2D(GL_RGBA, 256, 256, nullptr);

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}

    rgg::GetObserver()->projection = PerspectiveProjection();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    rgg::RenderTexture(texture,
                       math::Rect(0.0f, 0.0f, 256.0f, 256.0f),
                       math::Rect(0.0f, 0.0f, 50.0f, 50.0f));

    rgg::RenderText("TEST", 50.0f, 50.0f, math::Vec4f(1.f, 1.f, 1.f, 1.f));

    RenderGrid();

    window::SwapBuffers();
  }

  return 0;
}
