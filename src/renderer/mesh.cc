#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gl/gl.cc"

namespace rgg {

constexpr uint32_t kMaxMaterial = 16;

struct Material {
  char name[32];
  int illum;
  v3f kd;
  v3f ka;
  v3f tf;
  float ni;
  v3f ks;
  // Vert index first and end.
  uint32_t first = -1;
  uint32_t count;
};

struct Mesh {
  uint32_t vert_count = 0;
  uint32_t norm_count = 0;
  GLuint vert_vbo = 0;
  GLuint norm_vbo = 0;
  GLuint vao = 0;
  Material material[kMaxMaterial];
  uint32_t material_count = 0;
  bool IsValid() const { return vert_count > 0; }
};

constexpr int kMaxVertCount = 65538;
static float kVerts[kMaxVertCount * 8];
static uint32_t kVertElementCount;
static float kNorms[kMaxVertCount * 8];
static uint32_t kNormElementCount;

bool
LoadMTL(const char* filename, Material* material, uint32_t* material_count)
{
  FILE* f = fopen(filename, "rb");
  if (!f) {
    printf("%s not found!\n", filename);
    return false;
  }
  printf("Loading material: %s\n", filename);
  char line[1024];
  Material* cmat = nullptr;
  while (1) {
    int res = fscanf(f, "%s", line);
    if (res == EOF) break;
    if (strcmp(line, "newmtl") == 0) {
      if (*material_count >= kMaxMaterial) {
        printf("Material slots exhausted...\n");
        break;
      }
      cmat = &material[(*material_count)++];
      fscanf(f, "%s\n", &cmat->name);
    } else if (strcmp(line, "illum") == 0) {
      if (!cmat) continue;
      fscanf(f, "%i\n", &cmat->illum);
    } else if (strcmp(line, "Kd") == 0) {
      if (!cmat) continue;
      v3f* kd = &cmat->kd;
      fscanf(f, "%f %f %f\n", &kd->x, &kd->y, &kd->z);
    } else if (strcmp(line, "Ka") == 0) {
      if (!cmat) continue;
      v3f* ka = &cmat->ka;
      fscanf(f, "%f %f %f\n", &ka->x, &ka->y, &ka->z);
    } else if (strcmp(line, "Tf") == 0) {
      if (!cmat) continue;
      v3f* tf = &cmat->tf;
      fscanf(f, "%f %f %f\n", &tf->x, &tf->y, &tf->z);
    } else if (strcmp(line, "Ni") == 0) {
      if (!cmat) continue;
      fscanf(f, "%f\n", &cmat->ni);
    } else if (strcmp(line, "Ks") == 0) {
      if (!cmat) continue;
      v3f* ks = &cmat->ks;
      fscanf(f, "%f %f %f\n", &ks->x, &ks->y, &ks->z);
    }
  }

  for (int i = 0; i < *material_count; ++i) {
    cmat = &material[i];
    printf("Loaded Material %s\n", cmat->name);
    printf("  illum %i\n", cmat->illum);
    printf("  kd %.3f %.3f %.3f\n",
           cmat->kd.x, cmat->kd.y, cmat->kd.z);
    printf("  ka %.3f %.3f %.3f\n",
           cmat->ka.x, cmat->ka.y, cmat->ka.z);
    printf("  tf %.3f %.3f %.3f\n",
           cmat->tf.x, cmat->tf.y, cmat->tf.z);
    printf("  ni %.3f\n", cmat->ni);
    printf("  ks %.3f %.3f %.3f\n",
           cmat->ks.x, cmat->ks.y, cmat->ks.z);
  }
  return true;
}

bool
LoadOBJ(const char* filename, Mesh* mesh)
{
  *mesh = {};
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
  printf("Loading mesh: %s\n", filename);
  char line[1024];
  Material* mtl = nullptr;
  while (1) {
    int res = fscanf(f, "%s", line);
    if (res == EOF) break;
    if (strcmp(line, "v") == 0) {
      v3f* vert = &v[cv++];
      fscanf(f, "%f %f %f\n", &vert->x, &vert->y, &vert->z);
    } else if (strcmp(line, "vt") == 0) {
      continue;  // I don't use these yet.
    } else if (strcmp(line, "mtllib") == 0) {
      if (mesh->material_count < kMaxMaterial) {
        char mtlname[64] = {};
        fscanf(f, "%s\n", mtlname);
        if (filesystem::HasExtension(mtlname, "mtl")) {
          char fullname[128] = {};
          strcpy(fullname, filename);
          filesystem::ReplaceFilename(mtlname, fullname);
          if (!LoadMTL(fullname, mesh->material, &mesh->material_count)) {
            printf("Unable to load material %s\n", mtlname);
          }
        }
      }
    } else if (strcmp(line, "vn") == 0) {
      v3f* norm = &vn[cvn++];
      fscanf(f, "%f %f %f\n", &norm->x, &norm->y, &norm->z);
    } else if (strcmp(line, "vt") == 0) {
      continue;  // I don't use these yet.
    } else if (strcmp(line, "usemtl") == 0) {
      char mtlname[64] = {};
      fscanf(f, "%s\n", mtlname);
      for (int i = 0; i < mesh->material_count; ++i) {
        if (strcmp(mesh->material[i].name, mtlname) == 0) {
          mtl = &mesh->material[i];
        }
      }
    } else if (strcmp(line, "f") == 0) {
      if (mtl && mtl->first == uint32_t(-1)) {
        mtl->first = kVertElementCount / 3;
      }
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
      if (mtl) mtl->count = (kVertElementCount / 3) - mtl->first;
    }
  } 

  mesh->vert_count = kVertElementCount / 3;
  mesh->norm_count = kNormElementCount / 3;
  
  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, kVertElementCount * sizeof(GLfloat),
               &kVerts[0], GL_STATIC_DRAW);

  if (!points_vbo) {
    fclose(f);
    return false;
  }

  GLuint normals_vbo = 0;
  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, kNormElementCount * sizeof(GLfloat),
               &kNorms[0], GL_STATIC_DRAW);
  
  if (!normals_vbo) {
    fclose(f);
    return false;
  }

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  if (!vao) {
    fclose(f);
    return false;
  }

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

  fclose(f);

  return true;
}


}  // namespace rgg
