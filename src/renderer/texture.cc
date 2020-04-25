#pragma once

struct Texture {
  GLuint reference = 0;
  float width = 0.f;
  float height = 0.f;
  GLenum format;
  bool IsValid() const { return width > 0.f && height > 0.f; }
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
    -0.5f,  -0.5f, 0.0f, // BL
    -0.5f,  0.5f, 0.0f, // TL
    0.5f,  0.5f, 0.0f, // TR
    0.5f, -0.5f, 0.0f, // BR
    -0.5f,  -0.5f, 0.0f, // BL
    0.5f,  0.5f, 0.0f, // TR
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

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

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
  glGenerateMipmap(GL_TEXTURE_2D);
  texture.width = width;
  texture.height = height;
  texture.format = format;
  return texture;
}

bool
LoadTGA(const char* file, Texture* texture)
{
#pragma pack(push, 1)
  struct TgaImageSpec {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t image_width;
    uint16_t image_height;
    uint8_t pixel_depth;
    uint8_t image_descriptor;
  };
  struct TgaHeader {
    uint8_t id_length;
    uint8_t color_map_type;
    uint8_t image_type;
    uint8_t color_map_spec[5];
  };
#pragma pack(pop)

  FILE* fptr;
  uint8_t* buffer;
  uint32_t file_length;

  fptr = fopen(file, "rb");
  fseek(fptr, 0, SEEK_END);
  file_length = ftell(fptr);
  rewind(fptr);
  buffer = (uint8_t*)malloc(file_length);
  fread(buffer, file_length, 1, fptr);

  // First load the header.
  TgaHeader* header = (TgaHeader*)buffer;
  // Just don't even support colors.
  assert(header->id_length == 0);
  assert(header->color_map_type == 0);
  // Get the image_spec. This has overall image details.
  TgaImageSpec* image_spec = (TgaImageSpec*)(&buffer[sizeof(TgaHeader)]);
  // Only support 8-bit pixel depths.
  assert(image_spec->pixel_depth == 8);

#if 0
  printf("TGA file: %s header\n", file);
  printf("header->id_length: %i\n", header->id_length);
  printf("header->color_map_type: %i\n", header->color_map_type);
  printf("header->image_type: %i\n", header->image_type); 
  printf("TGA file: %s Image Spec\n", file);
  printf("image_spec->x_origin: %i\n", image_spec->x_origin);
  printf("image_spec->y_origin: %i\n", image_spec->y_origin);
  printf("image_spec->image_width: %i\n", image_spec->image_width);
  printf("image_spec->image_height: %i\n", image_spec->image_height);
  printf("image_spec->pixel_depth: %i\n", image_spec->pixel_depth);
  printf("image_spec->image_descriptor: %i\n", image_spec->image_descriptor);
#endif

  // Image bytes sz
  uint32_t image_bytes_size =
      image_spec->image_width * image_spec->image_height;
  uint8_t* image_bytes = &buffer[sizeof(TgaHeader) + sizeof(TgaImageSpec)];
  GLenum format = GL_RGBA;
  if (image_spec->pixel_depth == 8) format = GL_RED;
  else if (image_spec->pixel_depth == 24) format = GL_RGB;
  else {
    printf("Unsupported tga pixel depth\n");
    free(buffer);
    fclose(fptr);
    return false;
  }
  *texture = CreateTexture2D(format, image_spec->image_width,
                             image_spec->image_height, image_bytes);

  printf("Loaded texture %s\n", file);
  printf("size: %.0fx%.0f\n", texture->width, texture->height);
  printf("format: %s\n", gl::GLEnumToString(texture->format));
  printf("glreference: %i\n", texture->reference);

  // Free buffer used to read in file.
  free(buffer);
  fclose(fptr);
  return true;
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
RenderTexture(const Texture& texture, const Rectf& src,
              const Rectf& dest)
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
  uv[2] = {start_x + width, start_y + height}; // TR
  uv[3] = {start_x + width, start_y}; // BR
  uv[4] = {start_x, start_y}; // BL
  uv[5] = {start_x + width, start_y + height}; // TR
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
  v3f pos(dest.x + dest.width / 2.f, dest.y + dest.height / 2.f,0.0f);
  v3f scale(dest.width, dest.height, 1.f);
  Mat4f model = math::Model(pos, scale);
  Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniformMatrix4fv(kTextureState.matrix_uniform, 1, GL_FALSE, &matrix.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
