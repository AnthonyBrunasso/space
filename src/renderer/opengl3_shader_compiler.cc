#pragma once

namespace rgg
{
constexpr uint64_t length = 4096;
static char kBuffer[length];

uint64_t GLGetShaderInfoLog(uint32_t shader_reference, uint64_t length, char* log) {
  GLsizei actual_length = 0;
  glGetShaderInfoLog(shader_reference, length, &actual_length, log);
  return actual_length;
}

uint64_t GLGetProgramInfoLog(uint32_t program_reference, uint64_t length, char* log) {
  GLsizei actual_length = 0;
  glGetProgramInfoLog(program_reference, length, &actual_length, log);
  return actual_length;
}

bool GLCompileShader(GLenum shader_type, const GLchar* const* src, GLuint* id) {
  *id = glCreateShader(shader_type);
  glShaderSource(*id, 1, src, NULL);
  glCompileShader(*id);
  s32 params = -1;
  glGetShaderiv(*id, GL_COMPILE_STATUS, &params);
  if (params != GL_TRUE) {
    GLGetShaderInfoLog(*id, length, kBuffer);
    printf("%s\n", *src);
    printf("Shader Log: %s\n", kBuffer);
    return false;
  }
  return true;
}

// Link shader and check for errors if any exist.
bool GLLinkShaders(GLuint* id, int n, ...) {
  *id = glCreateProgram();
  va_list vl;
  va_start(vl, n);
  for (int i = 0; i < n; ++i) {
    glAttachShader(*id, va_arg(vl, GLuint));
  }
  va_end(vl);
  glLinkProgram(*id);
  int params = -1;
  glGetProgramiv(*id, GL_LINK_STATUS, &params);
  if (params != GL_TRUE) {
    GLGetProgramInfoLog(*id, length, kBuffer);
    printf("Program Log: %s\n", kBuffer);
    return false;
  }
  return true;
}

// Given a program id prints -
// LINK_STATUS, ATTACHED_SHADERS, ACTIVE_ATTRIBUTES, ACTIVE_UNIFORMS
void GLPrintProgramInfoString(GLuint program_reference) {
  printf("Program reference: %u\n", program_reference);
  s32 params = -1;
  glGetProgramiv(program_reference, GL_LINK_STATUS, &params);
  printf("GL_LINK_STATUS = %d\n", params);
  glGetProgramiv(program_reference, GL_ATTACHED_SHADERS, &params);
  printf("GL_ATTACHED_SHADERS = %u\n", params);
  glGetProgramiv(program_reference, GL_ACTIVE_ATTRIBUTES, &params);
  printf("GL_ACTIVE_ATTRIBUTES = %u\n", params);

  for (GLuint i = 0; i < (GLuint)params; i++) {
    char name[64];
    s32 max_length = 64;
    GLsizei actual_length = 0;
    s32 size = 0;
    GLenum type;
    glGetActiveAttrib(program_reference, i, max_length, &actual_length, &size,
                      &type, name);
    if (size > 1) {
      for (int j = 0; j < size; j++) {
        char long_name[64];
        std::snprintf(long_name, 64, "%s[%i]", name, j);
        s32 location = glGetAttribLocation(program_reference, long_name);
        printf("%d) type: %s name: %s location: %d\n", i, GLTypeToString(type),
               long_name, location);
      }
    } else {
      s32 location = glGetAttribLocation(program_reference, name);
      printf("%d) type: %s name: %s location: %d\n", i, GLTypeToString(type),
             name, location);
    }
  }
  glGetProgramiv(program_reference, GL_ACTIVE_UNIFORMS, &params);
  printf("GL_ACTIVE_UNIFORMS = %d\n", params);
  for (GLuint i = 0; i < (GLuint)params; i++) {
    char name[64];
    s32 max_length = 64;
    GLsizei actual_length = 0;
    s32 size = 0;
    GLenum type;
    glGetActiveUniform(program_reference, i, max_length, &actual_length, &size,
                       &type, name);
    if (size > 1) {
      for (int j = 0; j < size; j++) {
        char long_name[64];
        std::snprintf(long_name, 64, "%s[%i]", name, j);
        s32 location = glGetUniformLocation(program_reference, long_name);
        printf("%d) type: %s name: %s location: %d\n", i, GLTypeToString(type),
               long_name, location);
      }
    } else {
      s32 location = glGetUniformLocation(program_reference, name);
      printf("%d) type: %s name: %s location: %d\n", i, GLTypeToString(type),
             name, location);
    }
  }
}

}
