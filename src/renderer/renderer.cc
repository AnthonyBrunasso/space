#pragma once

#include "shader.h"
#include "tga_loader.cc"

#include "gl/gl.cc"
#include "math/math.cc"
#include "platform/platform.cc"

struct RenderTag {
  // TODO(abrunasso): Support custom shaders.
  GLuint vao_reference;
  GLuint vert_count;
  GLenum mode;
};

namespace rgg
{
struct GeometryProgram {
  GLuint reference = -1;
  GLuint matrix_uniform = -1;
  GLuint color_uniform = -1;
};

struct GeometryProgram3d {
  GLuint reference = -1;
  GLuint projection_uniform = -1;
  GLuint view_uniform = -1;
  GLuint model_uniform = -1;
  GLuint color_uniform = -1;
};


struct SmoothRectangleProgram {
  GLuint reference = -1;
  GLuint model_uniform = -1;
  GLuint view_projection_uniform = -1;
  GLuint color_uniform = -1;
  GLuint smoothing_radius_uniform = -1;
};

struct CircleProgram {
  GLuint reference = -1;
  GLuint model_uniform = -1;
  GLuint view_projection_uniform = -1;
  GLuint color_uniform = -1;
  GLuint inner_radius_uniform = -1;
  GLuint outer_radius_uniform = -1;
};

struct Observer {
  math::Mat4f projection;
  math::Mat4f view = math::Identity();
};

struct RGG {
  GeometryProgram geometry_program;
  GeometryProgram3d geometry_program_3d;
  SmoothRectangleProgram smooth_rectangle_program;
  CircleProgram circle_program;

  // References to vertex data on GPU.
  GLuint triangle_vao_reference;
  GLuint rectangle_vao_reference;
  GLuint line_vao_reference;
  GLuint line_vbo_reference;
  GLuint cube_vao_reference;

  int meter_size = 50;
};

static Observer kObserver;
static RGG kRGG;

#include "texture.cc"
#include "ui.cc"

class ModifyObserver
{
 public:
  ModifyObserver(const math::Mat4f& proj, const math::Mat4f& view)
  {
    projection_ = kObserver.projection;
    view_ = kObserver.view;
    kObserver.projection = proj;
    kObserver.view = view;
  }

  ~ModifyObserver()
  {
    kObserver.projection = projection_;
    kObserver.view = view_;
  }

 private:
  math::Mat4f projection_;
  math::Mat4f view_;
};

Observer*
GetObserver()
{
  return &kObserver;
}

bool
SetupGeometryProgram3d()
{
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &kVertexShader3d, &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kFragmentShader3d, &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.geometry_program_3d.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.geometry_program_3d.projection_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "projection");
  assert(kRGG.geometry_program_3d.projection_uniform != uint32_t(-1));
  kRGG.geometry_program_3d.view_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "view");
  assert(kRGG.geometry_program_3d.view_uniform != uint32_t(-1));
  kRGG.geometry_program_3d.model_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "model");
  assert(kRGG.geometry_program_3d.model_uniform != uint32_t(-1));
  //kRGG.geometry_program_3d.color_uniform =
  //    glGetUniformLocation(kRGG.geometry_program_3d.reference, "color");
  //assert(kRGG.geometry_program_3d.color_uniform != uint32_t(-1));
  return true;
}


bool
SetupGeometryProgram()
{
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &kVertexShader, &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kFragmentShader, &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.geometry_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  if (!gl::CompileShader(GL_VERTEX_SHADER, &kSmoothRectangleVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kSmoothRectangleFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.smooth_rectangle_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.geometry_program.matrix_uniform =
      glGetUniformLocation(kRGG.geometry_program.reference, "matrix");
  assert(kRGG.geometry_program.matrix_uniform != uint32_t(-1));
  kRGG.geometry_program.color_uniform =
      glGetUniformLocation(kRGG.geometry_program.reference, "color");
  assert(kRGG.geometry_program.color_uniform != uint32_t(-1));

  kRGG.smooth_rectangle_program.model_uniform =
      glGetUniformLocation(kRGG.smooth_rectangle_program.reference, "model");
  assert(kRGG.smooth_rectangle_program.model_uniform != uint32_t(-1));
  kRGG.smooth_rectangle_program.view_projection_uniform = glGetUniformLocation(
      kRGG.smooth_rectangle_program.reference, "view_projection");
  assert(kRGG.smooth_rectangle_program.view_projection_uniform != uint32_t(-1));

  kRGG.smooth_rectangle_program.color_uniform =
      glGetUniformLocation(kRGG.smooth_rectangle_program.reference, "color");
  assert(kRGG.smooth_rectangle_program.color_uniform != uint32_t(-1));
  kRGG.smooth_rectangle_program.smoothing_radius_uniform = glGetUniformLocation(
      kRGG.smooth_rectangle_program.reference, "smoothing_radius");
  assert(kRGG.smooth_rectangle_program.smoothing_radius_uniform !=
         uint32_t(-1));

  return true;
}

bool
SetupCircleProgram()
{
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &kCircleVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kCircleFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!gl::LinkShaders(&kRGG.circle_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  // No use for the basic shaders after the program is linked.
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.circle_program.model_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "model");
  assert(kRGG.circle_program.model_uniform != uint32_t(-1));
  kRGG.circle_program.view_projection_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "view_projection");
  assert(kRGG.circle_program.model_uniform != uint32_t(-1));

  kRGG.circle_program.color_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "color");
  kRGG.circle_program.inner_radius_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "inner_radius");
  assert(kRGG.circle_program.inner_radius_uniform != uint32_t(-1));
  kRGG.circle_program.outer_radius_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "outer_radius");
  assert(kRGG.circle_program.outer_radius_uniform != uint32_t(-1));
  kRGG.circle_program.color_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "color");
  assert(kRGG.circle_program.color_uniform != uint32_t(-1));
  return true;
}

bool
Initialize()
{
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  printf("Renderer: %s Version: %s\n", renderer, version);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);

  // Compile and link shaders.
  if (!SetupGeometryProgram()) return false;
  if (!SetupGeometryProgram3d()) return false;
  if (!SetupCircleProgram()) return false;

  // Create the geometry for basic shapes.
  float m = kRGG.meter_size;
  GLfloat tri[9] = {0.0f, m / 2.f,  0.f,      m / 2.f, -m / 2.f,
                    0.f,  -m / 2.f, -m / 2.f, 0.f};
  kRGG.triangle_vao_reference = gl::CreateGeometryVAO(9, tri);

  // Rectangle. Notice it's a square. Scale to make rectangly.
  // clang-format off
  static GLfloat square[18] = {
      -m / 2.f, m / 2.f, 0.f,
      m / 2.f, m / 2.f, 0.f,
      m / 2.f, -m / 2.f, 0.f,

      -m / 2.f, -m / 2.f, 0.f,
      -m / 2.f, m / 2.f, 0.f,
      m / 2.f, -m / 2.f, 0.f
  };
  // clang-format on
  kRGG.rectangle_vao_reference = gl::CreateGeometryVAO(18, square);

  // Don't use CreateGeometryVAO here because the vbo reference is used by
  // RenderLine to feed line endpoints directly to GPU.
  GLfloat line[6] = {-1.f, 0.f, 0.f, 1.f, 0.f, 0.f};
  glGenBuffers(1, &kRGG.line_vbo_reference);
  glBindBuffer(GL_ARRAY_BUFFER, kRGG.line_vbo_reference);
  glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), &line[0],
               GL_STATIC_DRAW);
  glGenVertexArrays(1, &kRGG.line_vao_reference);
  glBindVertexArray(kRGG.line_vao_reference);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, kRGG.line_vbo_reference);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  static GLfloat cube[36 * 3] = {
    -0.5f,-0.5f,-0.5f, // Left face
    -0.5f,-0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f,-0.5f,  // Back face
    -0.5f,-0.5f,-0.5f,
    -0.5f, 0.5f,-0.5f,
    0.5f,-0.5f, 0.5f,  // Bottom face
    -0.5f,-0.5f,-0.5f,
    0.5f,-0.5f,-0.5f,
    0.5f, 0.5f,-0.5f,  // Back face
    0.5f,-0.5f,-0.5f,
    -0.5f,-0.5f,-0.5f,
    -0.5f,-0.5f,-0.5f, // Left face
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f,-0.5f,
    0.5f,-0.5f, 0.5f,  // Bottom face
    -0.5f,-0.5f, 0.5f,
    -0.5f,-0.5f,-0.5f,
    -0.5f, 0.5f, 0.5f, // Front face
    -0.5f,-0.5f, 0.5f,
    0.5f,-0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,  // Right face
    0.5f,-0.5f,-0.5f,
    0.5f, 0.5f,-0.5f,
    0.5f,-0.5f,-0.5f,  // Right face
    0.5f, 0.5f, 0.5f,
    0.5f,-0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,  // Top face
    0.5f, 0.5f,-0.5f,
    -0.5f, 0.5f,-0.5f,
    0.5f, 0.5f, 0.5f,  // Top face
    -0.5f, 0.5f,-0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,  // Front face
    -0.5f, 0.5f, 0.5f,
    0.5f,-0.5f, 0.5f
  };

  static GLfloat cube_normals[36 * 3] = {
    -1.f, 0.f, 0.f, // Left face
    -1.f, 0.f, 0.f, // Left face
    -1.f, 0.f, 0.f, // Left face
    0.f, 0.f, -1.f, // Back face
    0.f, 0.f, -1.f, // Back face
    0.f, 0.f, -1.f, // Back face
    0.f, -1.f, 0.f, // Bottom face
    0.f, -1.f, 0.f, // Bottom face
    0.f, -1.f, 0.f, // Bottom face
    0.f, 0.f, -1.f, // Back face
    0.f, 0.f, -1.f, // Back face
    0.f, 0.f, -1.f, // Back face
    -1.f, 0.f, 0.f, // Left face
    -1.f, 0.f, 0.f, // Left face
    -1.f, 0.f, 0.f, // Left face
    0.f, -1.f, 0.f, // Bottom face
    0.f, -1.f, 0.f, // Bottom face
    0.f, -1.f, 0.f, // Bottom face
    0.f, 0.f, 1.f,  // Front face
    0.f, 0.f, 1.f,  // Front face
    0.f, 0.f, 1.f,  // Front face
    1.f, 0.f, 0.f,  // Right face
    1.f, 0.f, 0.f,  // Right face
    1.f, 0.f, 0.f,  // Right face
    1.f, 0.f, 0.f,  // Right face
    1.f, 0.f, 0.f,  // Right face
    1.f, 0.f, 0.f,  // Right face
    0.f, 1.f, 0.f,  // Top face
    0.f, 1.f, 0.f,  // Top face
    0.f, 1.f, 0.f,  // Top face
    0.f, 1.f, 0.f,  // Top face
    0.f, 1.f, 0.f,  // Top face
    0.f, 1.f, 0.f,  // Top face
    0.f, 0.f, 1.f,  // Front face
    0.f, 0.f, 1.f,  // Front face
    0.f, 0.f, 1.f,  // Front face
  };

  kRGG.cube_vao_reference = gl::CreateGeometryVAOWithNormals(
      36 * 3, cube, cube_normals);

  if (!SetupTexture()) {
    printf("Failed to setup Texture.\n");
    return false;
  }

  if (!SetupUI()) {
    printf("Failed to setup UI.\n");
    return false;
  }

  return true;
}

RenderTag
CreateRenderable(int vert_count, GLfloat* verts, GLenum mode)
{
  RenderTag tag = {};
  tag.vao_reference = gl::CreateGeometryVAO(vert_count * 3, verts);
  tag.vert_count = vert_count;
  tag.mode = mode;
  return tag;
}

void
RenderTag(const RenderTag& tag, const v3f& position, const v3f& scale,
          const math::Quatf& orientation, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(tag.vao_reference);
  // Translate and rotate the triangle appropriately.
  math::Mat4f model = math::Model(position, scale, orientation);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(tag.mode, 0, tag.vert_count);
}

void
RenderTriangle(const v3f& position, const v3f& scale,
               const math::Quatf& orientation, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.triangle_vao_reference);
  // Translate and rotate the triangle appropriately.
  math::Mat4f model = math::Model(position, scale, orientation);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_LINE_LOOP, 0, 3);
}

void
RenderRectangle(const v3f& position, const v3f& scale,
                const math::Quatf& orientation, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.rectangle_vao_reference);
  // Translate and rotate the rectangle appropriately.
  math::Mat4f model = math::Model(position, scale, orientation);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void
RenderRectangle(const math::Rectf& rect, float z, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  // Texture state has quad with length 1 geometry. This makes scaling simpler
  // as we can use the width / height directly in scale matrix.
  glBindVertexArray(kTextureState.vao_reference);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, z);
  v3f scale(rect.width, rect.height, 1.f);
  math::Mat4f model = math::Model(pos, scale);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void
RenderRectangle(const math::Rectf& rect, const v4f& color)
{
  RenderRectangle(rect, 0.f, color);
}

void
RenderLineRectangle(const math::Rectf& rect, float z, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  // Texture state has quad with length 1 geometry. This makes scaling simpler
  // as we can use the width / height directly in scale matrix.
  glBindVertexArray(kTextureState.vao_reference);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, z);
  v3f scale(rect.width, rect.height, 1.f);
  math::Mat4f model = math::Model(pos, scale);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void
RenderLineRectangle(const math::Rectf& rect, const v4f& color)
{
  RenderLineRectangle(rect, 0.f, color);
}

void
RenderSmoothRectangle(const math::Rectf& rect, float smoothing_radius,
                      const v4f& color)
{
  glUseProgram(kRGG.smooth_rectangle_program.reference);
  glBindVertexArray(kTextureState.vao_reference);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, 0.0f);
  v3f scale(rect.width, rect.height, 1.f);
  math::Mat4f model = math::Model(pos, scale);
  math::Mat4f view_projection = kObserver.projection * kObserver.view;
  glUniform1f(kRGG.smooth_rectangle_program.smoothing_radius_uniform,
              rect.width - smoothing_radius);
  glUniform4f(kRGG.smooth_rectangle_program.color_uniform, color.x, color.y,
              color.z, color.w);
  glUniformMatrix4fv(kRGG.smooth_rectangle_program.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniformMatrix4fv(kRGG.smooth_rectangle_program.view_projection_uniform, 1,
                     GL_FALSE, &view_projection.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void
RenderCircle(const v3f& position, float inner_radius, float outer_radius,
             const v4f& color)
{
  glUseProgram(kRGG.circle_program.reference);
  glBindVertexArray(kTextureState.vao_reference);
  // Translate and rotate the circle appropriately.
  math::Mat4f model =
      math::Model(position, v3f(outer_radius * 2.f, outer_radius * 2.f, 0.0f));
  math::Mat4f view_pojection = kObserver.projection * kObserver.view;
  glUniform1f(kRGG.circle_program.inner_radius_uniform, inner_radius);
  glUniform1f(kRGG.circle_program.outer_radius_uniform, outer_radius);
  glUniform4f(kRGG.circle_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.circle_program.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniformMatrix4fv(kRGG.circle_program.view_projection_uniform, 1, GL_FALSE,
                     &view_pojection.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void
RenderCircle(const v3f& position, float radius, const v4f& color)
{
  RenderCircle(position, 0.0f, radius, color);
}

void
RenderLine(const v3f& start, const v3f& end, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.line_vao_reference);
  // Model matrix unneeded here due to verts being set directly.
  math::Mat4f view_pojection = kObserver.projection * kObserver.view;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &view_pojection.data_[0]);
  float verts[6] = {start.x, start.y, start.z, end.x, end.y, end.z};
  glBindBuffer(GL_ARRAY_BUFFER, kRGG.line_vbo_reference);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_LINES, 0, 2);
}

void
RenderGrid(v2f grid, math::Rectf bounds, uint64_t color_count, v4f* color)
{
  // Prepare Geometry and color
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.line_vao_reference);

  // Draw horizontal lines.
  const v2f top_right(bounds.x + bounds.width, bounds.y + bounds.height);
  int i = 0;
  for (float y = bounds.y; y <= top_right.y; y += grid.y) {
    auto start = v3f(bounds.x, y, 0.f);
    auto end = v3f(top_right.x, y, 0.f);
    RenderLine(start, end, *color);
    i += 1;
    i = (i != color_count) * i;
  }

  // Draw vertical lines.
  i = 0;
  for (float x = bounds.x; x <= top_right.x; x += grid.x) {
    auto start = v3f(x, bounds.y, 0.f);
    auto end = v3f(x, top_right.y, 0.f);
    RenderLine(start, end, *color);
    i += 1;
    i = (i != color_count) * i;
  }
}

void
RenderCube(const math::Cubef& cube, const v4f& color)
{
  glUseProgram(kRGG.geometry_program_3d.reference);
  glBindVertexArray(kRGG.cube_vao_reference);
  v3f pos(cube.pos.x, cube.pos.y, cube.pos.z);
  v3f scale(cube.width, cube.height, cube.depth);
  math::Mat4f model = math::Model(pos, scale);
  glUniform4f(kRGG.geometry_program_3d.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program_3d.projection_uniform, 1, GL_FALSE,
                     &kObserver.projection.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.view_uniform, 1, GL_FALSE,
                     &kObserver.view.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
}

void
RenderLineCube(const math::Cubef& cube, const v4f& color)
{
  glUseProgram(kRGG.geometry_program.reference);
  math::Mat4f matrix = kObserver.projection * kObserver.view;
  glUniform4f(kRGG.geometry_program_3d.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  v3f back_top_left = cube.pos + v3f(0.f, cube.height, 0.f);
  v3f back_top_right = cube.pos + v3f(cube.width, cube.height, 0.f);
  v3f back_bottom_left = cube.pos;
  v3f back_bottom_right = cube.pos + v3f(cube.width, 0.f, 0.f);


  v3f front_top_left = cube.pos + v3f(0.f, cube.height, cube.depth);
  v3f front_top_right = cube.pos + v3f(cube.width, cube.height, cube.depth);
  v3f front_bottom_left = cube.pos + v3f(0.f, 0.f, cube.depth);;
  v3f front_bottom_right = cube.pos + v3f(cube.width, 0.f, cube.depth);

  // TODO(abrunasso): Probably a cheapear way to do this.

  // Draw back face
  RenderLine(back_bottom_left, back_top_left, color);
  RenderLine(back_bottom_left, back_bottom_right, color);
  RenderLine(back_bottom_right, back_top_right, color);
  RenderLine(back_top_left, back_top_right, color);

  // Connecting edges between back and front.
  RenderLine(back_bottom_left, front_bottom_left, color);
  RenderLine(back_bottom_right, front_bottom_right, color);
  RenderLine(back_top_left, front_top_left, color);
  RenderLine(back_top_right, front_top_right, color);

  // Draw front face.
  RenderLine(front_bottom_left, front_top_left, color);
  RenderLine(front_bottom_left, front_bottom_right, color);
  RenderLine(front_bottom_right, front_top_right, color);
  RenderLine(front_top_left, front_top_right, color);
}

}  // namespace rgg
