#pragma once

#include "asset/font.cc"

// HI! THIS IS IN THE rgg NAMESPACE.
// DONT INCLUDE IT ANYWHERE OUTSIDE OF renderer.cc

struct Font {
  Texture texture;
  GLuint program;
  GLuint texture_uniform;
  GLuint matrix_uniform;
  GLuint color_uniform;
  GLuint vbo;
  GLuint vao;
};

struct UI {
  Font font;
};

static UI kUI;

b8
SetupUI()
{
  Font& font = kUI.font;
  font.texture = CreateTexture2D(GL_RED, kFontWidth, kFontHeight,
                                 TextureInfo(), kFontData);
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
  font.color_uniform = glGetUniformLocation(font.program, "color");

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

s32
GetNextKerning(const char* msg, s32 msg_len, s32 first, s32 second)
{
  if (first == msg_len - 1) return 0;
  const FontMetadataRow* row = &kFontMetadataRow[msg[first]];
  for (s32 i = 0; i < row->kcount; ++i) {
    if (row->kerning[i].second == msg[second]) {
      return row->kerning[i].amount;
    }
  }
  return 0;
}

void
GetTextInfo(const char* msg, s32 msg_len, r32* width, r32* height,
            r32* min_y_offset)
{
  auto& font = kUI.font;
  *height = kFontLineHeight;
  *width = 0.0f;
  *min_y_offset = 1000.0f;
  s32 kerning_offset = 0;
  for (s32 i = 0; i < msg_len; ++i) {
    const FontMetadataRow* row = &kFontMetadataRow[msg[i]];
    *width += (r32)row->xadvance + kerning_offset;
    r32 y_offset = (r32)row->yoffset;
    if (y_offset < *min_y_offset) *min_y_offset = y_offset;
    kerning_offset = GetNextKerning(msg, msg_len, i, i + 1);
  }
}

Rectf
GetTextRect(const char* msg, s32 msg_len, v2f pos, r32 scale)
{
  r32 width, height, min_y_offset;
  GetTextInfo(msg, msg_len, &width, &height, &min_y_offset);
  return Rectf(
      pos.x, pos.y, width * scale, height * scale - min_y_offset * scale);
}

Rectf
GetTextRect(const char* msg, s32 msg_len, v2f pos)
{
  return GetTextRect(msg, msg_len, pos, 1.0f);
}

void
RenderText(const char* msg, v2f pos, r32 scale, const v4f& color)
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
  Mat4f projection = math::Ortho2(
      sz.x, 0.f, sz.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
  glUniformMatrix4fv(font.matrix_uniform, 1, GL_FALSE, &projection.data_[0]);
  glUniform4f(font.color_uniform, color.x, color.y, color.z, color.w);

#if 0
  math::Print4x4Matrix(kUI.projection);
  printf("\n");
#endif

  s32 msg_len = strlen(msg);
  s32 kerning_offset = 0;
  for (s32 i = 0; i < msg_len; ++i) {
    const FontMetadataRow* row = &kFontMetadataRow[msg[i]];
    //printf("%i\n", msg[i]);
    // The character trying to render is invalid. This means the font sheet
    // has no corresponding entry for the ascii id msg[i].
    // This could occur if you are attempting to render a '\n'
    assert(row->id != 0);

    // Scale to get uv coordinatoes.
    r32 tex_x = (r32)row->x / kFontWidth;
    r32 tex_y = (r32)row->y / kFontHeight;
    r32 tex_w = (r32)row->width / kFontWidth;
    r32 tex_h = (r32)row->height / kFontHeight;

    // Verts are subject to projection given by an orhthographic matrix
    // using the screen width and height.
    r32 v_w = (r32)row->width * scale;
    r32 v_h = (r32)row->height * scale;
    
    r32 offset_start_x =
      pos.x + (r32)row->xoffset * scale + (r32)kerning_offset * scale;
    r32 offset_start_y =
      pos.y - (r32)row->yoffset * scale + (r32)kFontLineHeight * scale;

#if 0
    printf("id=%i char=%c width=%i height=%i xoffset=%i yoffset=%i"
           " start_x=%.3f start_y=%.3f tex_w=%.3f tex_h=%.3f\n",
           msg[i], msg[i], row->width, row->height, row->xoffset,
           row->yoffset, offset_start_x, offset_start_y, tex_w, tex_h);
#endif

    TextPoint text_point[6] = {
      {offset_start_x, offset_start_y, tex_x, tex_y},
      {offset_start_x + v_w, offset_start_y, tex_x + tex_w, tex_y},
      {offset_start_x + v_w, offset_start_y - v_h, tex_x + tex_w, tex_y + tex_h},
      {offset_start_x + v_w, offset_start_y - v_h, tex_x + tex_w, tex_y + tex_h},
      {offset_start_x, offset_start_y - v_h, tex_x, tex_y + tex_h},
      {offset_start_x, offset_start_y, tex_x, tex_y}
    };

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
    pos.x += (r32)row->xadvance * scale + (r32)kerning_offset * scale;
    kerning_offset = GetNextKerning(msg, msg_len, i, i + 1);
#if 0
    printf("%c(%i) to %c(%i) offset:%i\n",
            msg[i], msg[i], msg[i + 1], msg[i + 1], kerning_offset);
#endif
  }
}

void
RenderText(const char* msg, v2f pos, const v4f& color)
{
  RenderText(msg, pos, 1.0f, color);
}

void
RenderButton(const char* text, const Rectf& rect, const v4f& color) {
  glUseProgram(kRGG.smooth_rectangle_program.reference);
  glBindVertexArray(kTextureState.vao_reference);
  v3f pos(rect.x + rect.width / 2.f, rect.y + rect.height / 2.f, 0.0f);
  v3f scale(rect.width, rect.height, 1.f);
  Mat4f model = math::Model(pos, scale);
  auto sz = window::GetWindowSize();
  Mat4f projection = math::Ortho2(
      sz.x, 0.f, sz.y, 0.f, /* 2d so leave near/far 0*/ 0.f, 0.f);
  Mat4f view_projection = projection;
  glUniform1f(kRGG.smooth_rectangle_program.smoothing_radius_uniform,
              rect.width - rect.width * 0.40f);
  glUniform4f(kRGG.smooth_rectangle_program.color_uniform, color.x, color.y,
              color.z, color.w);
  glUniformMatrix4fv(kRGG.smooth_rectangle_program.model_uniform, 1, GL_FALSE,
                     &model.data_[0]);
  glUniformMatrix4fv(kRGG.smooth_rectangle_program.view_projection_uniform, 1,
                     GL_FALSE, &view_projection.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
