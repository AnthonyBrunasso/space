#pragma once

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
  GLuint light_position_world_uniform = -1;
  GLuint suface_specular_uniform = -1;
  GLuint surface_diffuse_uniform = -1;
  GLuint surface_ambient_uniform = -1;
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
  Mat4f projection;
  Mat4f view = math::Identity();
  v3f position;
};

struct RGG {
  GeometryProgram geometry_program;
  GeometryProgram3d geometry_program_3d;
  SmoothRectangleProgram smooth_rectangle_program;
  CircleProgram circle_program;

  // References to vertex data on GPU.
  GLuint triangle_vao_reference;
  // TODO: Delete this. Use GL_DYANMIC_DRAW
  GLuint rectangle_vao_reference;
  GLuint line_vao_reference;
  GLuint line_vbo_reference;
  GLuint cube_vao_reference;
  GLuint cone_vao_reference;
  GLuint sphere_vao_reference;
  GLuint hex_vao_reference;

  s32 meter_size = 50;
};

enum DebugType {
  kDebugWorld,
  kDebugUI,
};

struct DebugSphere {
  v3f position;
  r32 radius;
  v4f color;
};

struct DebugCube {
  Cubef cube;
  v4f color;
  b8 fill = false;
};

struct DebugPoint {
  v3f position;
  r32 radius;
  v4f color;
  DebugType type;
};

struct DebugRect {
  Rectf rect;
  v4f color;
  DebugType type;
};

constexpr s32 kHexVertCount = 18;

DECLARE_ARRAY(DebugSphere, 8);
DECLARE_ARRAY(DebugCube, 128);
DECLARE_ARRAY(DebugPoint, 128);
DECLARE_ARRAY(DebugRect, 512);

static Observer kObserver;
static RGG kRGG;

#include "opengl3_texture.cc"
#include "texture_cache.cc"
#include "opengl3_ui.cc"
#include "camera.cc"

Mat4f DefaultPerspective(const v2f& dims, r32 fov = 64.f) {
  return math::Perspective(fov, dims.x / dims.y, 1.f, 1000.f);
}

Mat4f DefaultOrtho(const v2f& dims) {
  return math::Ortho(dims.x, 0.f, dims.y, 0.f, 2000.f, .1f);
}

class ModifyObserver
{
 public:
  ModifyObserver(const Mat4f& proj, const Mat4f& view)
  {
    projection_ = kObserver.projection;
    view_ = kObserver.view;
    kObserver.projection = proj;
    kObserver.view = view;
  }

  ModifyObserver(const rgg::Camera& camera)
  {
    projection_ = kObserver.projection;
    view_ = kObserver.view;
    kObserver.projection =
        math::Ortho(camera.viewport.x, 0.f, camera.viewport.y, 0.f, -1.f, 1.f);
    kObserver.view = math::LookAt(
        camera.position, camera.position + v3f(0.f, 0.f, 1.f), v3f(0.f, -1.f, 0.f));
  }

  ~ModifyObserver()
  {
    kObserver.projection = projection_;
    kObserver.view = view_;
  }

 private:
  Mat4f projection_;
  Mat4f view_;
};

Observer* GetObserver() {
  return &kObserver;
}

b8 SetupGeometryProgram3d() {
  GLuint vert_shader, frag_shader;
  if (!GLCompileShader(GL_VERTEX_SHADER, &kVertexShader3d, &vert_shader)) {
    return false;
  }

  if (!GLCompileShader(GL_FRAGMENT_SHADER, &kFragmentShader3d, &frag_shader)) {
    return false;
  }

  if (!GLLinkShaders(&kRGG.geometry_program_3d.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.geometry_program_3d.projection_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "projection");
  assert(kRGG.geometry_program_3d.projection_uniform != u32(-1));
  kRGG.geometry_program_3d.view_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "view");
  assert(kRGG.geometry_program_3d.view_uniform != u32(-1));
  kRGG.geometry_program_3d.model_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "model");
  assert(kRGG.geometry_program_3d.model_uniform != u32(-1));
  kRGG.geometry_program_3d.light_position_world_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "light_position_world");
  assert(kRGG.geometry_program_3d.light_position_world_uniform != u32(-1));
  kRGG.geometry_program_3d.suface_specular_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "surface_specular");
  assert(kRGG.geometry_program_3d.suface_specular_uniform != u32(-1));
  kRGG.geometry_program_3d.surface_diffuse_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "surface_diffuse");
  assert(kRGG.geometry_program_3d.surface_diffuse_uniform != u32(-1));
  kRGG.geometry_program_3d.surface_ambient_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "surface_ambient");
  assert(kRGG.geometry_program_3d.surface_ambient_uniform != u32(-1));

  kRGG.geometry_program_3d.color_uniform =
      glGetUniformLocation(kRGG.geometry_program_3d.reference, "color");
  //assert(kRGG.geometry_program_3d.color_uniform != u32(-1));
  return true;
}


b8 SetupGeometryProgram() {
  GLuint vert_shader, frag_shader;
  if (!GLCompileShader(GL_VERTEX_SHADER, &kVertexShader, &vert_shader)) {
    return false;
  }

  if (!GLCompileShader(GL_FRAGMENT_SHADER, &kFragmentShader, &frag_shader)) {
    return false;
  }

  if (!GLLinkShaders(&kRGG.geometry_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  if (!GLCompileShader(GL_VERTEX_SHADER, &kSmoothRectangleVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!GLCompileShader(GL_FRAGMENT_SHADER, &kSmoothRectangleFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!GLLinkShaders(&kRGG.smooth_rectangle_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.geometry_program.matrix_uniform =
      glGetUniformLocation(kRGG.geometry_program.reference, "matrix");
  assert(kRGG.geometry_program.matrix_uniform != u32(-1));
  kRGG.geometry_program.color_uniform =
      glGetUniformLocation(kRGG.geometry_program.reference, "color");
  assert(kRGG.geometry_program.color_uniform != u32(-1));

  kRGG.smooth_rectangle_program.model_uniform =
      glGetUniformLocation(kRGG.smooth_rectangle_program.reference, "model");
  assert(kRGG.smooth_rectangle_program.model_uniform != u32(-1));
  kRGG.smooth_rectangle_program.view_projection_uniform = glGetUniformLocation(
      kRGG.smooth_rectangle_program.reference, "view_projection");
  assert(kRGG.smooth_rectangle_program.view_projection_uniform != u32(-1));

  kRGG.smooth_rectangle_program.color_uniform =
      glGetUniformLocation(kRGG.smooth_rectangle_program.reference, "color");
  assert(kRGG.smooth_rectangle_program.color_uniform != u32(-1));
  kRGG.smooth_rectangle_program.smoothing_radius_uniform = glGetUniformLocation(
      kRGG.smooth_rectangle_program.reference, "smoothing_radius");
  assert(kRGG.smooth_rectangle_program.smoothing_radius_uniform !=
         u32(-1));

  return true;
}

b8 SetupCircleProgram() {
  GLuint vert_shader, frag_shader;
  if (!GLCompileShader(GL_VERTEX_SHADER, &kCircleVertexShader,
                         &vert_shader)) {
    return false;
  }

  if (!GLCompileShader(GL_FRAGMENT_SHADER, &kCircleFragmentShader,
                         &frag_shader)) {
    return false;
  }

  if (!GLLinkShaders(&kRGG.circle_program.reference, 2, vert_shader,
                       frag_shader)) {
    return false;
  }

  // No use for the basic shaders after the program is linked.
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  kRGG.circle_program.model_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "model");
  assert(kRGG.circle_program.model_uniform != u32(-1));
  kRGG.circle_program.view_projection_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "view_projection");
  assert(kRGG.circle_program.model_uniform != u32(-1));

  kRGG.circle_program.color_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "color");
  kRGG.circle_program.inner_radius_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "inner_radius");
  assert(kRGG.circle_program.inner_radius_uniform != u32(-1));
  kRGG.circle_program.outer_radius_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "outer_radius");
  assert(kRGG.circle_program.outer_radius_uniform != u32(-1));
  kRGG.circle_program.color_uniform =
      glGetUniformLocation(kRGG.circle_program.reference, "color");
  assert(kRGG.circle_program.color_uniform != u32(-1));
  return true;
}

void DebugReset() {
  kUsedDebugSphere = 0;
  kUsedDebugCube = 0;
  kUsedDebugPoint = 0;
  kUsedDebugRect = 0;
}

b8 Initialize() {
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  LOG(INFO, "Renderer: %s Version: %s", renderer, version);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_MULTISAMPLE);

  // Compile and link shaders.
  if (!SetupGeometryProgram()) return false;
  if (!SetupGeometryProgram3d()) return false;
  if (!SetupCircleProgram()) return false;

  // Create the geometry for basic shapes.
  r32 m = kRGG.meter_size;
  GLfloat tri[9] = {0.0f, m / 2.f,  0.f,      m / 2.f, -m / 2.f,
                    0.f,  -m / 2.f, -m / 2.f, 0.f};
  GLuint vbo;
  kRGG.triangle_vao_reference = GLCreateGeometryVAO(9, tri, &vbo);

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
  kRGG.rectangle_vao_reference = GLCreateGeometryVAO(18, square, &vbo);

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

  kRGG.cube_vao_reference = GLCreateGeometryVAOWithNormals(
      kCubeVertCount * 3, kCubeVerts, kCubeVertNorms);

  kRGG.cone_vao_reference = GLCreateGeometryVAOWithNormals(
      kConeVertCount * 3, kConeVerts, kConeVertNorms);
  
  kRGG.sphere_vao_reference = GLCreateGeometryVAOWithNormals(
      kSphereVertCount * 3, kSphereVerts, kSphereVertNorms);

  v2f hc[6];
  for (int i = 1; i < 7; ++i) {
    hc[i - 1] = math::HexCorner(v2f(0.f, 0.f), 5.f, i);
  }


  r32 kHexVerts[56] = {
    0.f, 0.f, 0.f, hc[0].x, hc[0].y, 0.f, hc[5].x, hc[5].y, 0.f,
    0.f, 0.f, 0.f, hc[5].x, hc[5].y, 0.f, hc[4].x, hc[4].y, 0.f,
    0.f, 0.f, 0.f, hc[4].x, hc[4].y, 0.f, hc[3].x, hc[3].y, 0.f,
    0.f, 0.f, 0.f, hc[3].x, hc[3].y, 0.f, hc[2].x, hc[2].y, 0.f,
    0.f, 0.f, 0.f, hc[2].x, hc[2].y, 0.f, hc[1].x, hc[1].y, 0.f,
    0.f, 0.f, 0.f, hc[1].x, hc[1].y, 0.f, hc[0].x, hc[0].y, 0.f,
  };

  r32 kHexVertNorms[56] = {
    0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
  };

  kRGG.hex_vao_reference = GLCreateGeometryVAOWithNormals(
      kHexVertCount * 3, kHexVerts, kHexVertNorms);

  if (!SetupTexture()) {
    LOG(WARN, "Failed to setup Texture.");
    return false;
  }

  if (!SetupUI()) {
    LOG(WARN, "Failed to setup UI.");
    return false;
  }

  if (!ImGuiImplSetup()) {
    LOG(WARN, "Failed to setup imgui.");
    return false;
  }

  return true;
}

RenderTag CreateRenderable(int vert_count, GLfloat* verts, GLenum mode) {
  RenderTag tag = {};
  GLuint vbo;
  tag.vao_reference = GLCreateGeometryVAO(vert_count * 3, verts, &vbo);
  tag.vert_count = vert_count;
  tag.mode = mode;
  return tag;
}

void RenderTag(const RenderTag& tag, const v3f& position, const v3f& scale,
               const Quatf& orientation, const v4f& color) {
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(tag.vao_reference);
  // Translate and rotate the triangle appropriately.
  Mat4f model = math::Model(position, scale, orientation);
  Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(tag.mode, 0, tag.vert_count);
}

void RenderTriangle(const v3f& position, const v3f& scale,
                    const Quatf& orientation, const v4f& color) {
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.triangle_vao_reference);
  // Translate and rotate the triangle appropriately.
  Mat4f model = math::Model(position, scale, orientation);
  Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_LINE_LOOP, 0, 3);
}

void RenderTriangle(const v2f& p, r32 half_height, const v4f& color) {
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kTextureState.vao_reference);
  Mat4f view_projection = kObserver.projection * kObserver.view;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z, color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &view_projection.data_[0]);
  v2f top = p + v2f(0.f, half_height);
  v2f bottom_right = p + v2f(half_height, -half_height);
  v2f bottom_left = p + v2f(-half_height, -half_height);
  r32 verts[9] = {
    top.x, top.y, 0.f,
    bottom_right.x, bottom_right.y, 0.f,
    bottom_left.x, bottom_left.y, 0.f,
  };
  glBindBuffer(GL_ARRAY_BUFFER, kTextureState.vbo_reference);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void RenderRectangle(const v3f& position, const v3f& scale,
                     const Quatf& orientation, const v4f& color) {
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.rectangle_vao_reference);
  // Translate and rotate the rectangle appropriately.
  Mat4f model = math::Model(position, scale, orientation);
  Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderRectangle(const Rectf& rect, r32 z, r32 rotation, const v4f& color) {
  glUseProgram(kRGG.geometry_program.reference);
  // Texture state has quad with length 1 geometry. This makes scaling simpler
  // as we can use the width / height directly in scale matrix.
  glBindVertexArray(kTextureState.vao_reference);
  Mat4f view_projection = kObserver.projection * kObserver.view;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &view_projection.data_[0]);
  math::Polygon<4> p = rect.Rotate(rotation);
  r32 verts[18] = {
    p.vertex[0].x, p.vertex[0].y, z,
    p.vertex[1].x, p.vertex[1].y, z,
    p.vertex[2].x, p.vertex[2].y, z,
    p.vertex[0].x, p.vertex[0].y, z,
    p.vertex[2].x, p.vertex[2].y, z,
    p.vertex[3].x, p.vertex[3].y, z,
  };
  glBindBuffer(GL_ARRAY_BUFFER, kTextureState.vbo_reference);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderRectangle(const Rectf& rect, const v4f& color) {
  RenderRectangle(rect, 0.f, 0.f, color);
}

void RenderLine(const v3f& start, const v3f& end, const v4f& color);

void RenderLineRectangle(const Rectf& rect, r32 z, r32 rotate, const v4f& color) {
  glUseProgram(kRGG.geometry_program.reference);
  // Texture state has quad with length 1 geometry. This makes scaling simpler
  // as we can use the width / height directly in scale matrix.
  glBindVertexArray(kTextureState.vao_reference);
  Mat4f view_projection = kObserver.projection * kObserver.view;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &view_projection.data_[0]);
  math::Polygon<4> p = rect.Rotate(rotate);
  r32 verts[12] = {
    p.vertex[0].x, p.vertex[0].y, z,
    p.vertex[1].x, p.vertex[1].y, z,
    p.vertex[2].x, p.vertex[2].y, z,
    p.vertex[3].x, p.vertex[3].y, z,
  };
  glBindBuffer(GL_ARRAY_BUFFER, kTextureState.vbo_reference);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void RenderLineRectangle(const Rectf& rect, const v4f& color) {
  RenderLineRectangle(rect, 0.f, 0.f, color);
}

void RenderSmoothRectangle(const Rectf& rect, r32 smoothing_radius, const v4f& color) {
  glUseProgram(kRGG.smooth_rectangle_program.reference);
  glBindVertexArray(kTextureState.vao_reference_static);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, 0.0f);
  v3f scale(rect.width, rect.height, 1.f);
  Mat4f model = math::Model(pos, scale);
  Mat4f view_projection = kObserver.projection * kObserver.view;
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

void RenderCircle(const v3f& position, r32 inner_radius, r32 outer_radius, const v4f& color) {
  glUseProgram(kRGG.circle_program.reference);
  glBindVertexArray(kTextureState.vao_reference_static);
  // Translate and rotate the circle appropriately.
  Mat4f model =
      math::Model(position, v3f(outer_radius * 2.f, outer_radius * 2.f, 0.0f));
  Mat4f view_pojection = kObserver.projection * kObserver.view;
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

void RenderCircle(const v3f& position, r32 radius, const v4f& color) {
  RenderCircle(position, 0.0f, radius, color);
}

void RenderLine(const v3f& start, const v3f& end, const v4f& color) {
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.line_vao_reference);
  // Model matrix unneeded here due to verts being set directly.
  Mat4f view_pojection = kObserver.projection * kObserver.view;
  glUniform4f(kRGG.geometry_program.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &view_pojection.data_[0]);
  r32 verts[6] = {start.x, start.y, start.z, end.x, end.y, end.z};
  glBindBuffer(GL_ARRAY_BUFFER, kRGG.line_vbo_reference);
  glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
  glDrawArrays(GL_LINES, 0, 2);
}

void RenderGrid(const v2f& grid, const Rectf& bounds, uint64_t color_count, v4f* color) {
  // Prepare Geometry and color
  glUseProgram(kRGG.geometry_program.reference);
  glBindVertexArray(kRGG.line_vao_reference);

  // Draw horizontal lines.
  const v2f top_right(bounds.x + bounds.width, bounds.y + bounds.height);
  s32 i = 0;
  for (r32 y = bounds.y; y <= top_right.y; y += grid.y) {
    auto start = v3f(bounds.x, y, 0.f);
    auto end = v3f(top_right.x, y, 0.f);
    RenderLine(start, end, *color);
    i += 1;
    i = (i != color_count) * i;
  }

  // Draw vertical lines.
  i = 0;
  for (r32 x = bounds.x; x <= top_right.x; x += grid.x) {
    auto start = v3f(x, bounds.y, 0.f);
    auto end = v3f(x, top_right.y, 0.f);
    RenderLine(start, end, *color);
    i += 1;
    i = (i != color_count) * i;
  }
}
void SetDefaultSurfaceMaterial() {
  // Set some reasonable lighting defaults.
  glUniform3f(kRGG.geometry_program_3d.suface_specular_uniform,
              1.f, 1.f, 1.f);
  glUniform3f(kRGG.geometry_program_3d.surface_diffuse_uniform,
              .7f, .7f, .8f);
  glUniform3f(kRGG.geometry_program_3d.surface_ambient_uniform,
              1.f, 1.f, 1.f);
}

void Render3d(const v3f& pos, const v3f& scale, const v4f& color, GLuint vao, s32 verts) {
  glUseProgram(kRGG.geometry_program_3d.reference);
  glBindVertexArray(vao);
  Mat4f model = math::Model(pos, scale);
  glUniform4f(kRGG.geometry_program_3d.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program_3d.projection_uniform, 1, GL_FALSE,
                     &kObserver.projection.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.view_uniform, 1, GL_FALSE,
                     &kObserver.view.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniform3f(kRGG.geometry_program_3d.light_position_world_uniform,
              kObserver.position.x, kObserver.position.y,
              kObserver.position.z);
  SetDefaultSurfaceMaterial();
  glDrawArrays(GL_TRIANGLES, 0, verts);
}

void Render3d(const v3f& pos, const v3f& scale, const v4f& color, GLuint vao, s32 verts, const Mat4f& perspective) {
  glUseProgram(kRGG.geometry_program_3d.reference);
  glBindVertexArray(vao);
  Mat4f model = math::Model(pos, scale);
  glUniform4f(kRGG.geometry_program_3d.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program_3d.projection_uniform, 1, GL_FALSE,
                     &perspective.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.view_uniform, 1, GL_FALSE,
                     &kObserver.view.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniform3f(kRGG.geometry_program_3d.light_position_world_uniform,
              kObserver.position.x, kObserver.position.y,
              kObserver.position.z);
  SetDefaultSurfaceMaterial();
  glDrawArrays(GL_TRIANGLES, 0, verts);
}

void Render3dWithRotation(const v3f& pos, const v3f& scale, const Quatf& quat, const v4f& color, GLuint vao, s32 verts) {
  glUseProgram(kRGG.geometry_program_3d.reference);
  glBindVertexArray(vao);
  Mat4f model = math::Model(pos, scale, quat);
  glUniform4f(kRGG.geometry_program_3d.color_uniform, color.x, color.y, color.z,
              color.w);
  glUniformMatrix4fv(kRGG.geometry_program_3d.projection_uniform, 1, GL_FALSE,
                     &kObserver.projection.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.view_uniform, 1, GL_FALSE,
                     &kObserver.view.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniform3f(kRGG.geometry_program_3d.light_position_world_uniform,
              kObserver.position.x, kObserver.position.y,
              kObserver.position.z);
  SetDefaultSurfaceMaterial();
  glDrawArrays(GL_TRIANGLES, 0, verts);
}

void RenderMesh(const Mesh& mesh, const Mat4f& model, const v4f& color = v4f(1.f, 1.f, 1.f, 1.f)) {
  if (kRGG.geometry_program_3d.color_uniform != u32(-1)) {
    glUniform4f(kRGG.geometry_program_3d.color_uniform, color.x, color.y,
                color.z, color.w);
  }
  glUniformMatrix4fv(kRGG.geometry_program_3d.projection_uniform, 1, GL_FALSE,
                     &kObserver.projection.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.view_uniform, 1, GL_FALSE,
                     &kObserver.view.data_[0]);
  glUniformMatrix4fv(kRGG.geometry_program_3d.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniform3f(kRGG.geometry_program_3d.light_position_world_uniform,
              kObserver.position.x, kObserver.position.y,
              kObserver.position.z);
  if (!mesh.material_count) {
    SetDefaultSurfaceMaterial();
    glDrawArrays(GL_TRIANGLES, 0, mesh.vert_count);
  } else {
    for (s32 i = 0; i < mesh.material_count; ++i) {
      const Material* mat = &mesh.material[i];
      glUniform3f(kRGG.geometry_program_3d.suface_specular_uniform,
                  mat->ks.x, mat->ks.y, mat->ks.z);
      glUniform3f(kRGG.geometry_program_3d.surface_diffuse_uniform,
                  mat->kd.x, mat->kd.y, mat->kd.z);
      glUniform3f(kRGG.geometry_program_3d.surface_ambient_uniform,
                  mat->ka.x, mat->ka.y, mat->ka.z);
      for (s32 j = 0; j < mat->vert_pair_count; ++j) {
        // Set surface lighting properties.
        const MaterialVertPair* vp = &mat->vert_pair[j];
        glDrawArrays(GL_TRIANGLES, vp->first, vp->count);
      }
    }
  }
}

// Leaving color as default will only use lighting properties.
void RenderMesh(const Mesh& mesh, const v3f& pos, const v3f& scale, const Quatf& quat, const v4f& color = v4f(1.f, 1.f, 1.f, 1.f)) {
  if (!mesh.IsValid()) return;
  glUseProgram(kRGG.geometry_program_3d.reference);
  glBindVertexArray(mesh.vao);
  Mat4f model = math::Model(pos, scale, quat);
  RenderMesh(mesh, model, color);
}

void RenderMesh(const Mesh& mesh, const v3f& pos, const v3f& scale,
           r32 x_rotation, r32 y_rotation, r32 z_rotation,
           const v4f& color = v4f(1.f, 1.f, 1.f, 1.f)) {
  if (!mesh.IsValid()) return;
  glUseProgram(kRGG.geometry_program_3d.reference);
  glBindVertexArray(mesh.vao);
  Mat4f model = math::Model(pos, scale) * math::RotationX(x_rotation) *
                math::RotationY(y_rotation) * math::RotationZ(z_rotation);
  RenderMesh(mesh, model, color);
}

void RenderCube(const Cubef& cube, const v4f& color) {
  Render3d(cube.pos, v3f(cube.width, cube.height, cube.depth), color,
           kRGG.cube_vao_reference, kCubeVertCount);
}

void RenderCube(const Cubef& cube, const v4f& color, const Mat4f& perspective) {
  Render3d(cube.pos, v3f(cube.width, cube.height, cube.depth), color,
           kRGG.cube_vao_reference, kCubeVertCount, perspective);
}

void RenderCube(const Cubef& cube, const Quatf& orientation, const v4f& color) {
  Render3dWithRotation(cube.pos, v3f(cube.width, cube.height, cube.depth),
                       orientation, color, kRGG.cube_vao_reference,
                       kCubeVertCount);
}

void RenderHexagon(const v3f& pos, const v3f& scale, const v4f& color) {
  Render3d(pos, scale, color, kRGG.hex_vao_reference, kHexVertCount);
}

void RenderCone(v3f pos, v3f scale, const v4f& color) {
  Render3d(pos, scale, color, kRGG.cone_vao_reference, kConeVertCount);
}

void RenderSphere(v3f pos, v3f scale, const v4f& color) {
  Render3d(pos, scale, color, kRGG.sphere_vao_reference, kSphereVertCount);
}

void RenderLineCube(const Cubef& cube, const v4f& color) {
  glUseProgram(kRGG.geometry_program.reference);
  Mat4f matrix = kObserver.view * kObserver.projection;
  glUniform4f(kRGG.geometry_program_3d.color_uniform, color.x, color.y,
              color.z, color.w);
  glUniformMatrix4fv(kRGG.geometry_program.matrix_uniform, 1, GL_FALSE,
                     &matrix.data_[0]);
  v3f pos =
      cube.pos - v3f(cube.width / 2.f, cube.height / 2.f, cube.depth / 2.f);
  v3f back_top_left = pos + v3f(0.f, cube.height, 0.f);
  v3f back_top_right = pos + v3f(cube.width, cube.height, 0.f);
  v3f back_bottom_left = pos;
  v3f back_bottom_right = pos + v3f(cube.width, 0.f, 0.f);


  v3f front_top_left = pos + v3f(0.f, cube.height, cube.depth);
  v3f front_top_right = pos + v3f(cube.width, cube.height, cube.depth);
  v3f front_bottom_left = pos + v3f(0.f, 0.f, cube.depth);;
  v3f front_bottom_right = pos + v3f(cube.width, 0.f, cube.depth);

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

void RenderLineHexagon(const v3f& center, r32 size, const v4f& color) {
  v3f points[6];
  for (s32 i = 1; i <= 6; ++i) {
    points[i - 1] = v3f(math::HexCorner(center.xy(), 5.f, i));
    points[i - 1].z = center.z;
  }
  RenderLine(points[5], points[0], color);
  RenderLine(points[0], points[1], color);
  RenderLine(points[1], points[2], color);
  RenderLine(points[2], points[3], color);
  RenderLine(points[3], points[4], color);
  RenderLine(points[4], points[5], color);
}

void RenderProgressBar(const Rectf& rect, r32 z, r32 current_progress,
                  r32 max_progress, const v4f& fill_color,
                  const v4f& outline_color) {
  if (current_progress > FLT_EPSILON) {
    Rectf fill_rect(
        rect.x, rect.y,
        rect.width * fmodf(current_progress, max_progress) / max_progress,
        rect.height);
    if (current_progress == max_progress) fill_rect.width = rect.width;
    RenderRectangle(fill_rect, z, 0.f, fill_color);
  }
  RenderLineRectangle(rect, z, 0.f, outline_color);
}

void DebugRenderWorldPrimitives() {
  // Perspetive / world debugging.
  for (s32 i = 0; i < kUsedDebugSphere; ++i) {
    r32 r = kDebugSphere[i].radius;
    rgg::RenderSphere(kDebugSphere[i].position, v3f(r, r, r),
                      kDebugSphere[i].color);
  }

  for (s32 i = 0; i < kUsedDebugCube; ++i) {
    if (kDebugCube[i].fill) {
      rgg::RenderCube(kDebugCube[i].cube, kDebugCube[i].color);
    } else {
      rgg::RenderLineCube(kDebugCube[i].cube, kDebugCube[i].color);
    }
  }

  for (s32 i = 0; i < kUsedDebugPoint; ++i) {
    if (kDebugPoint[i].type != kDebugWorld) continue;
    DebugPoint* point = &kDebugPoint[i];
    rgg::RenderCircle(point->position, point->radius, point->color);
  }

  for (s32 i = 0; i < kUsedDebugRect; ++i) {
    if (kDebugRect[i].type != kDebugWorld) continue;
    DebugRect* rect = &kDebugRect[i];
    //printf("...\n");
    rgg::RenderLineRectangle(rect->rect, rect->color);
  }
}

void DebugRenderUIPrimitives() {
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_ONE, GL_ZERO);
  // Orthographic / UI debugging
  auto dims = window::GetWindowSize();
  rgg::ModifyObserver mod(math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f),
                          math::Identity());

  for (s32 i = 0; i < kUsedDebugPoint; ++i) {
    if (kDebugPoint[i].type != kDebugUI) continue;
    DebugPoint* point = &kDebugPoint[i];
    rgg::RenderCircle(point->position, point->radius, point->color);
  }

  for (s32 i = 0; i < kUsedDebugRect; ++i) {
    if (kDebugRect[i].type != kDebugUI) continue;
    DebugRect* rect = &kDebugRect[i];
    rgg::RenderLineRectangle(rect->rect, rect->color);
  }
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
}

void DebugRenderPrimitives() {
  DebugRenderWorldPrimitives();
  DebugRenderUIPrimitives();
}

void DebugPushSphere(const v3f& position, r32 radius, const v4f& color) {
  DebugSphere* dsphere = UseDebugSphere();
  dsphere->position = position;
  dsphere->radius = radius;
  dsphere->color = color;
} 

void DebugPushCube(const Cubef& cube, const v4f& color, b8 fill) {
  DebugCube* dcube = UseDebugCube();
  dcube->cube = cube;
  dcube->color = color;
  dcube->fill = fill;
}

void DebugPushCube(const Cubef& cube, const v4f& color) {
  DebugPushCube(cube, color, false);
}

void DebugPushPoint(const v3f& position, r32 radius, const v4f& color, DebugType type) {
  DebugPoint* dpoint = UseDebugPoint();
  dpoint->position = position;
  dpoint->radius = radius;
  dpoint->color = color;
  dpoint->type = type;
}

void DebugPushPoint(const v3f& position, r32 radius, const v4f& color) {
  DebugPushPoint(position, radius, color, kDebugWorld);
}


void DebugPushRect(const Rectf& rect, const v4f& color, DebugType type) {
  DebugRect* drect = UseDebugRect();
  drect->rect = rect;
  drect->color = color;
  drect->type = type;
}


void DebugPushRect(const Rectf& rect, const v4f& color) {
  DebugPushRect(rect, color, kDebugWorld);
}

}  // namespace rgg
