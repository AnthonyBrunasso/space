#define GL_ASSERT_INFO()                  \
  {                                       \
    GLuint err = glGetError();            \
    if (err > 0) {                        \
      LOG(ERR, "glGetError() = %u", err); \
      assert(err == 0);                   \
    }                                     \
  }                                       

namespace rgg
{
const char* GLTypeToString(int type) {
  switch (type) {
    case GL_BOOL:
      return "bool";
    case GL_INT:
      return "int";
    case GL_FLOAT:
      return "float";
    case GL_FLOAT_VEC2:
      return "vec2";
    case GL_FLOAT_VEC3:
      return "vec3";
    case GL_FLOAT_VEC4:
      return "vec4";
    case GL_FLOAT_MAT2:
      return "mat2";
    case GL_FLOAT_MAT3:
      return "mat3";
    case GL_FLOAT_MAT4:
      return "mat4";
    case GL_SAMPLER_2D:
      return "sampler2D";
    case GL_SAMPLER_3D:
      return "sampler3D";
    case GL_SAMPLER_CUBE:
      return "samplerCube";
    case GL_SAMPLER_2D_SHADOW:
      return "sampler2DShadow";
    default:
      break;
  }
  return "other";
}

const char* GLEnumToString(int type) {
  switch (type) {
    case GL_RED:
      return "GL_RED";
    case GL_RGB:
      return "GL_RGB";
    case GL_RGBA:
      return "GL_RGBA";
    default:
      break;
  }
  return "Unknown";
}

uint32_t GLCreateGeometryVAO(int len, GLfloat* verts, GLuint* points_vbo) {
  // Create points VBO.
  *points_vbo = 0;
  glGenBuffers(1, points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, *points_vbo);
  glBufferData(GL_ARRAY_BUFFER, len * sizeof(GLfloat), &verts[0],
               GL_STATIC_DRAW);
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, *points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  return vao;
}

uint32_t GLCreateGeometryVAOWithNormals(int len, GLfloat* verts, GLfloat* normals) {
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, len * sizeof(GLfloat), &verts[0],
               GL_STATIC_DRAW);

  GLuint normals_vbo = 0;
  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, len * sizeof(GLfloat), &normals[0],
               GL_STATIC_DRAW);

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  return vao;
}

}
