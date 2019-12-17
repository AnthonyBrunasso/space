#include "gfx.h"

#include <iostream>

#include "ecs.h"
#include "math/vec.h"
#include "math/mat_ops.h"
#include "renderer/gl_utils.h"
#include "renderer/gl_shader_cache.h"

namespace gfx {

constexpr const char* kVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 matrix;
  void main() {
    gl_Position = matrix * vec4(vertex_position, 1.0);
  }
)";

constexpr const char* kVertexShaderName = "vert";

constexpr const char* kFragmentShader = R"(
  #version 410
	out vec4 frag_color;
  void main() {
   frag_color = vec4(1.0, 1.0, 1.0, 1.0);
  }
)";

constexpr const char* kFragmentShaderName = "frag";

constexpr const char* kProgramName = "prog";

struct Gfx {
  GLFWwindow* glfw = nullptr;
  renderer::GLShaderCache shader_cache;

  // References to shader programs.
  uint32_t program_reference;

  // References to uniforms.
  uint32_t matrix_uniform;

  // References to vertex data on GPU.
  uint32_t triangle_vao_reference;
  uint32_t rectangle_vao_reference;

  // State the mouse click is currently and previously in.
  int current_mouse_state;
  int previous_mouse_state;

  // Number of pixels that correspond with a meter.
  int meter_size = 50;
};

static Gfx kGfx;

bool Initialize() {
  kGfx.glfw = renderer::InitGLAndCreateWindow(800, 800, "Space");
  if (!kGfx.glfw) {
    std::cout << "Unable to start GL and create window."
              << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.CompileShader(
        kVertexShaderName, renderer::ShaderType::VERTEX, kVertexShader)) {
    std::cout << "Unable to compile " << kVertexShaderName << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.CompileShader(
      kFragmentShaderName, renderer::ShaderType::FRAGMENT, kFragmentShader)) {
    std::cout << "Unable to compile " << kFragmentShaderName << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.LinkProgram(
        kProgramName, { kVertexShaderName, kFragmentShaderName })) {
    std::cout << "Unable to link: " << kProgramName << " info: "
              << kGfx.shader_cache.GetProgramInfo(kProgramName)
              << std::endl;
    return false;
  }

  if (!kGfx.shader_cache.GetProgramReference(
        kProgramName, &kGfx.program_reference)) {
    return false;
  }

  kGfx.matrix_uniform =
      glGetUniformLocation(kGfx.program_reference, "matrix");

  // Create the geometry for basic shapes.

  // Triangle.
  float m = kGfx.meter_size;
  kGfx.triangle_vao_reference = renderer::CreateGeometryVAO({
      math::Vec2f( 0.0f,  m),
      math::Vec2f(    m, -m),
      math::Vec2f(   -m, -m)});

  // Rectangle. Notice it's a square. Scale to make rectangly.
  kGfx.rectangle_vao_reference = renderer::CreateGeometryVAO({
      math::Vec2f(-m / 2.f,  m / 2.f),
      math::Vec2f( m / 2.f,  m / 2.f),
      math::Vec2f( m / 2.f, -m / 2.f),
      math::Vec2f(-m / 2.f, -m / 2.f)});

  return true;
}

void PollEvents() {
  glfwPollEvents();
  int state = glfwGetMouseButton(kGfx.glfw, GLFW_MOUSE_BUTTON_LEFT);
  kGfx.previous_mouse_state = kGfx.current_mouse_state;
  kGfx.current_mouse_state = state;
}

bool Render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  math::Vec2f dims = GetWindowDims();
  // TODO: Take into consideration camera.
  math::Mat4f ortho = math::CreateOrthographicMatrix<float>(
      dims.x, 0.f, 0.f, dims.y, /* 2d so leave near/far 0*/ 0.f, 0.f);
  
  // Draw all triangles
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.triangle_vao_reference);
  kECS.Enumerate<TransformComponent, TriangleComponent>(
      [&](ecs::Entity entity,
          TransformComponent& transform, TriangleComponent&) {
    // Translate and rotate the triangle appropriately.
    math::Mat4f model = math::CreateTranslationMatrix(transform.position) *
                        math::CreateRotationMatrix(transform.orientation);
    math::Mat4f matrix = ortho * model;
    glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 3);
  });

  // Draw all rectangles.
  glUseProgram(kGfx.program_reference);
  glBindVertexArray(kGfx.rectangle_vao_reference);
  kECS.Enumerate<TransformComponent, RectangleComponent>(
      [&](ecs::Entity entity,
          TransformComponent& transform, RectangleComponent&) {
    // Translate and rotate the rectangle appropriately.
    math::Mat4f model = math::CreateTranslationMatrix(transform.position) *
                        math::CreateRotationMatrix(transform.orientation);
    math::Mat4f matrix = ortho * model;
    glUniformMatrix4fv(kGfx.matrix_uniform, 1, GL_FALSE, &matrix[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
  });

  glfwSwapBuffers(kGfx.glfw);
  return !glfwWindowShouldClose(kGfx.glfw);
}

math::Vec2f GetCursorPositionInScreenSpace() {
  double xpos, ypos;
  glfwGetCursorPos(kGfx.glfw, &xpos, &ypos);
  math::Vec2f dims = GetWindowDims();
  return math::Vec2f((float)xpos, (float)ypos);
}

math::Vec2f GetCursorPositionInWorldSpace() {
  return GetCursorPositionInScreenSpace() / kGfx.meter_size;
}

math::Vec2f GetWindowDims() {
  int width, height;
  glfwGetWindowSize(kGfx.glfw, &width, &height);
  return math::Vec2f((float)width, (float)height);
}


bool LeftMouseClicked() {
  return kGfx.current_mouse_state == GLFW_RELEASE &&
         kGfx.previous_mouse_state == GLFW_PRESS;
}

}
