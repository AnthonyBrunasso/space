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

    auto dims = window::GetWindowSize();

    rgg::GetObserver()->projection = Projection(dims);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderGrid();
#if 0
id=72 char=H width=17 height=20 xoffset=1 yoffset=6 start_x=640.000 start_y=386.000 tex_w=0.066 tex_h=0.078
id=101 char=e width=15 height=15 xoffset=0 yoffset=11 start_x=657.000 start_y=381.000 tex_w=0.059 tex_h=0.059
id=108 char=l width=4 height=20 xoffset=1 yoffset=6 start_x=672.000 start_y=386.000 tex_w=0.016 tex_h=0.078
id=108 char=l width=4 height=20 xoffset=1 yoffset=6 start_x=676.000 start_y=386.000 tex_w=0.016 tex_h=0.078
id=111 char=o width=15 height=15 xoffset=0 yoffset=11 start_x=680.000 start_y=381.000 tex_w=0.059 tex_h=0.059
id=44 char=, width=4 height=6 xoffset=2 yoffset=24 start_x=695.000 start_y=368.000 tex_w=0.016 tex_h=0.023
id=32 char=  width=3 height=1 xoffset=-1 yoffset=31 start_x=699.000 start_y=361.000 tex_w=0.012 tex_h=0.004
id=116 char=t width=8 height=20 xoffset=0 yoffset=6 start_x=702.000 start_y=386.000 tex_w=0.031 tex_h=0.078
id=104 char=h width=13 height=20 xoffset=1 yoffset=6 start_x=710.000 start_y=386.000 tex_w=0.051 tex_h=0.078
id=101 char=e width=15 height=15 xoffset=0 yoffset=11 start_x=723.000 start_y=381.000 tex_w=0.059 tex_h=0.059
id=114 char=r width=9 height=15 xoffset=1 yoffset=11 start_x=738.000 start_y=381.000 tex_w=0.035 tex_h=0.059
id=101 char=e width=15 height=15 xoffset=0 yoffset=11 start_x=747.000 start_y=381.000 tex_w=0.059 tex_h=0.059
#endif
    const char* msg = "Hello, there";
    
    v2f pos(dims.x / 2.f, dims.y / 2.f);
    int msg_len = strlen(msg);
    float offset, max_y, min_y;
    rgg::GetTextHeightInfo(msg, msg_len, &offset, &max_y, &min_y);
    float width = rgg::GetTextWidth(msg, msg_len);

    //printf("(%.2f, %.2f)\n", width, offset);

    imui::Text(msg, pos.x, pos.y);

    rgg::GetObserver()->projection =
      math::Ortho2(
          dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f);

    rgg::RenderLineRectangle(math::Rect(pos.x, pos.y, width, fmax(offset - min_y, max_y)),
                             v4f(1.0f, 0.0f, 0.0f, 1.0f));

    rgg::GetObserver()->projection = Projection(window::GetWindowSize());

    imui::Render();

    imui::Reset();

    window::SwapBuffers();
  }

  return 0;
}

