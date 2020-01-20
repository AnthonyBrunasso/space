#pragma once

namespace gl
{

// Helper for compiling shaders that prints out errors if any exist.
bool CompileShader(unsigned int shader_type, const char* const* src, unsigned int* id);

// Link shader and check for errors if any exist.
bool LinkShaders(unsigned int* id, int n, ...);

}  // namespace gl
