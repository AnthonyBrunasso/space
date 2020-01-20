#pragma once

#include "shader.h"

#include <cstdio>

#include "utils.h"

#include "platform/platform.cc"

namespace gl
{
constexpr uint64_t length = 4096;
static char buffer[length];

uint64_t
GetShaderInfoLog(uint32_t shader_reference, uint64_t length, char log[length])
{
  int actual_length = 0;
  glGetShaderInfoLog(shader_reference, length, &actual_length, log);
  return actual_length;
}

uint64_t
GetProgramInfoLog(uint32_t program_reference, uint64_t length, char log[length])
{
  int actual_length = 0;
  glGetProgramInfoLog(program_reference, length, &actual_length, log);
  return actual_length;
}

bool
CompileShader(GLenum shader_type, const GLchar* const* src, GLuint* id)
{
  *id = glCreateShader(shader_type);
  glShaderSource(*id, 1, src, NULL);
  glCompileShader(*id);
  int params = -1;
  glGetShaderiv(*id, GL_COMPILE_STATUS, &params);
  if (params != GL_TRUE) {
    GetShaderInfoLog(*id, length, buffer);
    printf("Shader Log: %s\n", buffer);
    return false;
  }
  return true;
}

// Link shader and check for errors if any exist.
bool
LinkShaders(GLuint* id, int n, ...)
{
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
    GetProgramInfoLog(*id, length, buffer);
    printf("Program Log: %s\n", buffer);
    return false;
  }
  return true;
}

// Given a program id prints -
// LINK_STATUS, ATTACHED_SHADERS, ACTIVE_ATTRIBUTES, ACTIVE_UNIFORMS
void
PrintProgramInfoString(GLuint program_reference)
{
  assert(0);
  printf("Program reference: %u\n", program_reference);
  int params = -1;
  glGetProgramiv(program_reference, GL_LINK_STATUS, &params);
  printf("GL_LINK_STATUS = %d\n", params);
  glGetProgramiv(program_reference, GL_ATTACHED_SHADERS, &params);
  printf("GL_ATTACHED_SHADERS = %u\n", params);
  glGetProgramiv(program_reference, GL_ACTIVE_ATTRIBUTES, &params);
  printf("GL_ACTIVE_ATTRIBUTES = %u\n", params);

  for (GLuint i = 0; i < (GLuint)params; i++) {
    char name[64];
    int max_length = 64;
    int actual_length = 0;
    int size = 0;
    GLenum type;
    glGetActiveAttrib(program_reference, i, max_length, &actual_length, &size,
                      &type, name);
    if (size > 1) {
      for (int j = 0; j < size; j++) {
        char long_name[64];
        std::sprintf(long_name, "%s[%i]", name, j);
        int location = glGetAttribLocation(program_reference, long_name);
        printf("%d) type: %s name: %s location: %d ", i, GLTypeToString(type),
               long_name, location);
      }
    } else {
      int location = glGetAttribLocation(program_reference, name);
      printf("%d) type: %s name: %s location: %d ", i, GLTypeToString(type),
             name, location);
    }
  }
  glGetProgramiv(program_reference, GL_ACTIVE_UNIFORMS, &params);
  printf("GL_ACTIVE_UNIFORMS = %d\n", params);
  for (GLuint i = 0; i < (GLuint)params; i++) {
    char name[64];
    int max_length = 64;
    int actual_length = 0;
    int size = 0;
    GLenum type;
    glGetActiveUniform(program_reference, i, max_length, &actual_length, &size,
                       &type, name);
    if (size > 1) {
      for (int j = 0; j < size; j++) {
        char long_name[64];
        std::sprintf(long_name, "%s[%i]", name, j);
        int location = glGetUniformLocation(program_reference, long_name);
        printf("%d) type: %s name: %s location: %d ", i, GLTypeToString(type),
               long_name, location);
      }
    } else {
      int location = glGetUniformLocation(program_reference, name);
      printf("%d) type: %s name: %s location: %d ", i, GLTypeToString(type),
             name, location);
    }
  }

}

}  // namespace gl
