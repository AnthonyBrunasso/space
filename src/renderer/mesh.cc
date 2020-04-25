#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gl/gl.cc"

namespace rgg {

struct Mesh {
  uint32_t vert_count = 0;
  uint32_t norm_count = 0;
  GLuint vert_vbo = 0;
  GLuint norm_vbo = 0;
  GLuint vao = 0;
  bool IsValid() const { return vert_count > 0; }
};

constexpr int kMaxVertCount = 65538;
static float kVerts[kMaxVertCount * 8];
static uint32_t kVertElementCount;
static float kNorms[kMaxVertCount * 8];
static uint32_t kNormElementCount;

bool
LoadOBJ(const char* filename, Mesh* mesh)
{
  static v3f v[kMaxVertCount];
  static uint32_t cv = 0;
  static v3f vn[kMaxVertCount];
  static uint32_t cvn = 0;

  cv = 0;
  cvn = 0;
  kVertElementCount = 0;
  kNormElementCount = 0;

#define ADD_VERTS(vec) \
  memcpy(&kVerts[kVertElementCount], &v[vec.x - 1].x, 3 * sizeof(float));  \
  memcpy(&kNorms[kNormElementCount], &vn[vec.z - 1].x, 3 * sizeof(float)); \
  kVertElementCount += 3;                                                  \
  kNormElementCount += 3;

#define ADD_VERT_AND_NORM(a, b, c) \
  ADD_VERTS(a);                    \
  ADD_VERTS(b);                    \
  ADD_VERTS(c);

  FILE* f = fopen(filename, "rb");
  if (!f) {
    printf("%s not found!\n", filename);
    return false;
  }
  printf("Loading %s\n", filename);
  char line[1024];
  while (1) {
    int res = fscanf(f, "%s", line);
    if (res == EOF) break;
    if (strcmp(line, "v") == 0) {
      v3f* vert = &v[cv++];
      fscanf(f, "%f %f %f\n", &vert->x, &vert->y, &vert->z);
    } else if (strcmp(line, "vt") == 0) {
      continue;  // I don't use these yet.
    } else if (strcmp(line, "vn") == 0) {
      v3f* norm = &vn[cvn++];
      fscanf(f, "%f %f %f\n", &norm->x, &norm->y, &norm->z);
    } else if (strcmp(line, "vt") == 0) {
      continue;  // I don't use these yet.
    } else if (strcmp(line, "f") == 0) {
      int i = 0;
      v3i first, second, third;
      fscanf(f, " %i/%i/%i", &first.x, &first.y, &first.z);
      fscanf(f, " %i/%i/%i", &second.x, &second.y, &second.z);
      fscanf(f, " %i/%i/%i", &third.x, &third.y, &third.z);
      ADD_VERT_AND_NORM(first, second, third);
      second = third;
      char c = fgetc(f);
      while (c != '\n') {
        fscanf(f, " %i/%i/%i", &third.x, &third.y, &third.z);
        ADD_VERT_AND_NORM(first, second, third);
        second = third;
        c = fgetc(f);
      }
    }
  } 

  mesh->vert_count = kVertElementCount / 3;
  mesh->norm_count = kNormElementCount / 3;
  
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, kVertElementCount * sizeof(GLfloat),
               &kVerts[0], GL_STATIC_DRAW);

  if (!points_vbo) return false;

  GLuint normals_vbo = 0;
  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, kNormElementCount * sizeof(GLfloat),
               &kNorms[0], GL_STATIC_DRAW);
  
  if (!normals_vbo) return false;

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  if (!vao) return false;

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  mesh->vert_vbo = points_vbo;
  mesh->norm_vbo = normals_vbo;
  mesh->vao = vao;

  printf("Loaded Mesh vert count %i verts vbo %i norms vbo %i vao %i \n",
         mesh->vert_count, mesh->vert_vbo, mesh->norm_vbo, mesh->vao);

  return true;
}


}  // namespace rgg
