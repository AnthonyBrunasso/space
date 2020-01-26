#pragma once

// HI! THIS IS IN THE rgg NAMESPACE.
// DONT INCLUDE IT ANYWHERE OUTSIDE OF renderer.cc

struct Font {
  uint16_t texture_width;
  uint16_t texture_height;
  FntMetadata metadata;
  Texture texture;
  GLuint program;
  GLuint texture_uniform;
  GLuint matrix_uniform;
  GLuint vbo;
  GLuint vao;
};

struct UI {
  Font font;
};

static UI kUI;

bool
SetupUI()
{
  Font& font = kUI.font;
  uint8_t* image_data;
  uint16_t texture_width, texture_height;
  LoadTGA("example/gfx/characters_0.tga", &image_data, &font.texture_width,
          &font.texture_height);
  font.metadata = LoadFntMetadata("example/gfx/characters.fnt");
  font.texture = CreateTexture2D(GL_RED, font.texture_width,
                                 font.texture_height, image_data);
  free(image_data); // GL has the texture data now. Free it from process.
  GLuint vert_shader, frag_shader;
  if (!gl::CompileShader(GL_VERTEX_SHADER, &kFontVertexShader,
                         &vert_shader)) {
    return false;
  }
  if (!gl::CompileShader(GL_FRAGMENT_SHADER, &kFontFragmentShader,
                         &frag_shader)) {
    return false;
  }
  if (!gl::LinkShaders(&font.program, 2, vert_shader, frag_shader)) {
    return false;
  }
  font.texture_uniform = glGetUniformLocation(font.program, "basic_texture");
  font.matrix_uniform = glGetUniformLocation(font.program, "matrix");

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  // Setup font vbo / vao.
  glGenBuffers(1, &font.vbo);

  glGenVertexArrays(1, &font.vao);
  glBindVertexArray(font.vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, font.vbo);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

  return true;
}

bool
__IsUseless(char c)
{
  // These characters would cause a high vertical height since y is positive
  // down in the sheet. Ignore them in calculating our height.
  return c == ' ' || c == '.' || c == '_' || c == '-';
}

void
GetTextInfo(const char* msg, int msg_len, float* offset,
            float* max_height, float* min_height, float* width)
{
  auto& font = kUI.font;
  *offset = -1000.0f;
  *min_height = 1000.0f;
  *max_height = -1000.0f;
  *width = 0.0f;
  for (int i = 0; i < msg_len; ++i) {
    const FntMetadataRow* row = &font.metadata.rows[msg[i]];
    *width += (float)row->width;
    if (__IsUseless(msg[i])) continue;
    float offset_y = (float)row->yoffset + (float)row->height;
    if (offset_y > *offset) *offset = offset_y;
    float y = (float)row->height;
    if (y < *min_height) *min_height = y;
    if (y > *max_height) *max_height = y;
  }
}

math::Rect
GetTextRect(const char* msg, int msg_len, float x, float y)
{
  float offset, max_y, min_y, width;
  rgg::GetTextInfo(msg, msg_len, &offset, &max_y, &min_y, &width);
  return math::Rect(x, y, width, fmax(offset - min_y, max_y));
}

void
RenderText(const char* msg, float x, float y, const v4f& color)
{
  auto& font = kUI.font;

  struct TextPoint {
    GLfloat x;
    GLfloat y;
    GLfloat u;
    GLfloat v;
    void Pr() { printf("%.1f,%.1f,%.1f,%.1f\n", x, y, u, v); }
  };

  glUseProgram(font.program);
  glBindVertexArray(font.vao);
  glBindTexture(GL_TEXTURE_2D, font.texture.reference);

  auto sz = window::GetWindowSize();
  math::Mat4f projection = math::Ortho2(
      sz.x, 0.f, sz.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
  glUniformMatrix4fv(font.matrix_uniform, 1, GL_FALSE, &projection[0]);

#if 0
  math::Print4x4Matrix(kUI.projection);
  printf("\n");
#endif

  int msg_len = strlen(msg);
  float offset, max_height, min_height, width;
  GetTextInfo(msg, msg_len, &offset, &max_height, &min_height, &width);
  for (int i = 0; i < msg_len; ++i) {
    TextPoint text_point[6];

    const FntMetadataRow* row = &font.metadata.rows[msg[i]];

    // The character trying to render is invalid. This means the font sheet
    // has no corresponding entry for the ascii id msg[i].
    // This could occur if you are attempting to render a '\n'
    assert(row->id != 0);

    // Scale to get uv coordinatoes.
    float tex_x = (float)row->x / font.texture_width;
    float tex_y = (float)row->y / font.texture_height;
    float tex_w = (float)row->width / font.texture_width;
    float tex_h = (float)row->height / font.texture_height;

    // Verts are subject to projection given by an orhthographic matrix
    // using the screen width and height.
    float v_w = (float)row->width;
    float v_h = (float)row->height;

    float offset_start_x = x/* - row->xoffset*/;
    float offset_start_y = y - row->yoffset + offset;

#if 0
    printf("id=%i char=%c width=%i height=%i xoffset=%i yoffset=%i"
           " start_x=%.3f start_y=%.3f tex_w=%.3f tex_h=%.3f\n",
           msg[i], msg[i], row->width, row->height, row->xoffset,
           row->yoffset, offset_start_x, offset_start_y, tex_w, tex_h);
#endif

    text_point[0] = {offset_start_x, offset_start_y, tex_x, tex_y};
    text_point[1] = {offset_start_x + v_w, offset_start_y, tex_x + tex_w,
                     tex_y};
    text_point[2] = {offset_start_x + v_w, offset_start_y - v_h,
                     tex_x + tex_w, tex_y + tex_h};
    text_point[3] = {offset_start_x + v_w, offset_start_y - v_h,
                     tex_x + tex_w, tex_y + tex_h};
    text_point[4] = {offset_start_x, offset_start_y - v_h, tex_x,
                     tex_y + tex_h};
    text_point[5] = {offset_start_x, offset_start_y, tex_x, tex_y};

#if 0
    text_point[0].Pr();
    text_point[1].Pr();
    text_point[2].Pr();
    text_point[3].Pr();
    text_point[4].Pr();
    text_point[5].Pr();
#endif
    glBindBuffer(GL_ARRAY_BUFFER, font.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(text_point), text_point, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);  // Draw the character 
    x += v_w;
  }
}

void
RenderButton(const char* text, const math::Rect& rect,
             const v4f& color) {
  glUseProgram(kRGG.smooth_rectangle_program.reference);
  glBindVertexArray(kTextureState.vao_reference);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, 0.0f);
  v3f scale(rect.width, rect.height, 1.f);
  math::Mat4f model = math::Model(pos, scale);
  auto sz = window::GetWindowSize();
  math::Mat4f projection = math::Ortho2(
      sz.x, 0.f, sz.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
  math::Mat4f view_projection = projection;
  glUniform1f(kRGG.smooth_rectangle_program.smoothing_radius_uniform,
              rect.width - rect.width * 0.40f);
  glUniform4f(kRGG.smooth_rectangle_program.color_uniform, color.x, color.y,
              color.z, color.w);
  glUniformMatrix4fv(kRGG.smooth_rectangle_program.model_uniform, 1, GL_FALSE,
                     &model[0]);
  glUniformMatrix4fv(kRGG.smooth_rectangle_program.view_projection_uniform, 1,
                     GL_FALSE, &view_projection[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
