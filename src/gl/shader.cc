#pragma once

#include "shader.h"

#include <cstdio>
#include <sstream>
#include <string>

#include "utils.h"

#include "platform/platform.cc"

namespace gl
{
std::string
GetShaderInfoLog(uint32_t shader_reference)
{
  constexpr int log_length = 4096;
  int actual_length = 0;
  char log[log_length];
  glGetShaderInfoLog(shader_reference, log_length, &actual_length, log);
  return "Shader Log for reference: " + std::to_string(shader_reference) +
         " log: " + log;
}

std::string
GetProgramInfoLog(uint32_t program_reference)
{
  constexpr int log_length = 4096;
  int actual_length = 0;
  char log[log_length];
  glGetProgramInfoLog(program_reference, log_length, &actual_length, log);
  return "Program Log for reference: " + std::to_string(program_reference) +
         " log: " + log;
}

// namespace

bool
CompileShader(GLenum shader_type, const GLchar* const* src, GLuint* id)
{
  *id = glCreateShader(shader_type);
  glShaderSource(*id, 1, src, NULL);
  glCompileShader(*id);
  int params = -1;
  glGetShaderiv(*id, GL_COMPILE_STATUS, &params);
  if (params != GL_TRUE) {
    printf("Shader Log: %s\n", GetShaderInfoLog(*id).c_str());
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
    printf("Program Log: %s\n", GetProgramInfoLog(*id).c_str());
    return false;
  }
  return true;
}

// Given a program id returns -
// LINK_STATUS, ATTACHED_SHADERS, ACTIVE_ATTRIBUTES, ACTIVE_UNIFORMS
// as a string. Useful for debugging.
const char*
AllocProgramInfoString(GLuint program_reference)
{
  assert(0);
  std::stringstream ss;
  ss << "Program reference: " << program_reference << std::endl;
  int params = -1;
  glGetProgramiv(program_reference, GL_LINK_STATUS, &params);
  ss << "GL_LINK_STATUS = " << params << std::endl;
  glGetProgramiv(program_reference, GL_ATTACHED_SHADERS, &params);
  ss << "GL_ATTACHED_SHADERS = " << params << std::endl;
  glGetProgramiv(program_reference, GL_ACTIVE_ATTRIBUTES, &params);
  ss << "GL_ACTIVE_ATTRIBUTES = " << params << std::endl;
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
        ss << i << ") type: " << GLTypeToString(type) << " name: " << long_name
           << " location: " << location << std::endl;
      }
    } else {
      int location = glGetAttribLocation(program_reference, name);
      ss << i << ") type: " << GLTypeToString(type) << " name: " << name
         << " location: " << location << std::endl;
    }
  }
  glGetProgramiv(program_reference, GL_ACTIVE_UNIFORMS, &params);
  ss << "GL_ACTIVE_UNIFORMS = " << params << std::endl;
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
        ss << i << ") type: " << GLTypeToString(type) << " name: " << long_name
           << " location: " << location << std::endl;
      }
    } else {
      int location = glGetUniformLocation(program_reference, name);
      ss << i << ") type: " << GLTypeToString(type) << " name: " << name
         << " location: " << location << std::endl;
    }
  }
  return strdup(ss.str().c_str());
}

}  // namespace gl
