#include <iostream>
#include "platform/platform.cc"

#include "math/mat.h"

const char* vertex_shader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 matrix_uniform;
  uniform vec4 color_uniform;
  out vec4 color; 
  void main() {
    color = color_uniform; 
    gl_Position = matrix_uniform * vec4(vertex_position, 1.0);
  }
)";

const char* fragment_shader = R"(
  #version 410
  in vec4 color;
	out vec4 frag_color;
  void main() {
   frag_color = color;
  }
)";

int
main()
{
  window::Create("Colored triangle", 640, 480);
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << renderer << std::endl;
  std::cout << version << std::endl;
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  // Enable alpha transparency.
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glEnable(GL_CULL_FACE);  // cull face
  glCullFace(GL_BACK);     // cull back face
  glFrontFace(GL_CW);      // GL_CCW for counter clock-wise

  GLfloat points[] = {
      0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f,
  };
  math::Mat4f matrix{
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.5f, 0.0f, 0.0f, 1.0f, // translation matrix.
  };
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Create points VBO.
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // This needs to be done for each bound vao
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  // Create and compile vertex shader.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  // Create and compile fragment shader.
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);
  // Link them together.
  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  int matrix_location = -1;
  matrix_location = glGetUniformLocation(shader_program, "matrix_uniform");
  std::cout << "matrix_location: " << matrix_location << std::endl;

  int color_location = -1;
  color_location = glGetUniformLocation(shader_program, "color_uniform");
  std::cout << "color_location: " << color_location << std::endl;

  v4f color(1.f, 0.f, 1.f, 0.25f);

  while (!window::ShouldClose()) {
    PlatformEvent event;
    while (window::PollEvent(&event)) {}
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Set the shader program.
    glUseProgram(shader_program);
    glUniform4f(color_location, color.x, color.y, color.z, color.w);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, &matrix[0]);
    // Bind the vertex array object.
    glBindVertexArray(vao);
    // glPolygonMode(GL_FRONT, GL_LINE);
    // Draw them points.
    glDrawArrays(GL_TRIANGLES, 0, 3);  // Draws the triangle
    window::SwapBuffers();
  }
  return 0;
}
