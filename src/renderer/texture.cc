#pragma once

struct Texture {
  GLuint reference;
  float width;
  float height;
};

struct TextureState {
  GLuint vao_reference;
  GLuint program;
  GLuint texture_uniform;
  GLuint matrix_uniform;
  GLuint uv_vbo;
  GLuint frame_buffer = -1;
};

struct UV {
  float u;
  float v;
};

static TextureState kTextureState;

bool
SetupTexture()
{
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &kTextureVertexShader,
                         &vert_shader)) {
    return false;
  }
  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kTextureFragmentShader,
                         &frag_shader)) {
    return false;
  }
  if (!gl::LinkShaders(&kTextureState.program, 2, vert_shader, frag_shader)) {
    return false;
  }
  kTextureState.texture_uniform =
      glGetUniformLocation(kTextureState.program, "basic_texture");
  kTextureState.matrix_uniform =
      glGetUniformLocation(kTextureState.program, "matrix");

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

   GLfloat quad[18] = {
    -0.5f,  -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
    0.5f,  0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
  };

  kTextureState.vao_reference = gl::CreateGeometryVAO(18, quad);
  glEnableVertexAttribArray(1);
  glGenBuffers(1, &kTextureState.uv_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, kTextureState.uv_vbo);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  return true;
}

Texture CreateTexture2D(GLenum format, uint64_t width, uint64_t height,
                        const void* data) {
  Texture texture = {};
  glGenTextures(1, &texture.reference);
  glBindTexture(GL_TEXTURE_2D, texture.reference);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    format,
    width,
    height,
    0,
    format,
    GL_UNSIGNED_BYTE,
    data
  );
  texture.width = width;
  texture.height = height;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  return texture;
}

void
BeginRenderTo(const Texture& texture)
{
  // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
  if (kTextureState.frame_buffer == GLuint(-1)) {
    glGenFramebuffers(1, &kTextureState.frame_buffer);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, kTextureState.frame_buffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.reference, 0);
  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);
  glBindFramebuffer(GL_FRAMEBUFFER, kTextureState.frame_buffer);
  glViewport(0, 0, texture.width, texture.height);
}

void
EndRenderTo()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  auto dims = window::GetWindowSize();
  glViewport(0, 0, dims.x, dims.y);
}

void
RenderTexture(const Texture& texture, const math::Rect& src,
              const math::Rect& dest)
{
  glUseProgram(kTextureState.program);
  glBindTexture(GL_TEXTURE_2D, texture.reference);
  glBindVertexArray(kTextureState.vao_reference);
  UV uv[6];
  // Match uv coordinates to quad coords.
  float start_x = src.x / texture.width;
  float start_y = src.y / texture.height;
  float width = src.width / texture.width;
  float height = src.height / texture.height;
  uv[0] = {start_x, start_y}; // BL
  uv[1] = {start_x, start_y + height}; // TL
  uv[2] = {start_x + width, start_y}; // BR
  uv[3] = {start_x, start_y + height}; // TL
  uv[4] = {start_x + width, start_y + height}; // TR
  uv[5] = {start_x + width, start_y}; // BR
#if 0
  printf("uv[0]=(%.3f, %3.f)\n", uv[0].u, uv[0].v);
  printf("uv[1]=(%.3f, %3.f)\n", uv[1].u, uv[1].v);
  printf("uv[2]=(%.3f, %3.f)\n", uv[2].u, uv[2].v);
  printf("uv[3]=(%.3f, %3.f)\n", uv[3].u, uv[3].v);
  printf("uv[4]=(%.3f, %3.f)\n", uv[4].u, uv[4].v);
  printf("uv[5]=(%.3f, %3.f)\n", uv[5].u, uv[5].v);
#endif
  glBindBuffer(GL_ARRAY_BUFFER, kTextureState.uv_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_DYNAMIC_DRAW);
  math::Vec3f pos(dest.x + dest.width / 2.f, dest.y + dest.height / 2.f,0.0f);
  math::Vec3f scale(dest.width, dest.height, 1.f);
  math::Mat4f model = math::Model(pos, scale);
  math::Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniformMatrix4fv(kTextureState.matrix_uniform, 1, GL_FALSE, &matrix[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
