#pragma once

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gl/gl.cc"
#include "memory/memory.cc"

namespace rgg {

#define DEBUGOBJ 0

constexpr u32 kMaxMaterial = 16;
constexpr u32 kMaxVertPair = 32;

struct MaterialVertPair {
  u64 first = u64(-1);
  u64 count;
};

struct Material {
  char name[32];
  s32 illum;
  v3f kd;
  v3f ka;
  v3f tf;
  r32 ni;
  v3f ks;
  MaterialVertPair vert_pair[kMaxVertPair];
  u64 vert_pair_count = 0;
};

struct Mesh {
  u32 vert_count = 0;
  u32 norm_count = 0;
  GLuint vert_vbo = 0;
  GLuint norm_vbo = 0;
  GLuint vao = 0;
  Material material[kMaxMaterial];
  u32 material_count = 0;
  bool IsValid() const { return vert_count > 0; }
};

constexpr s32 kMaxVertCount = 65538;

bool
LoadMTL(const char* filename, Material* material, u32* material_count)
{
  *material = {};
  FILE* f = fopen(filename, "rb");
  if (!f) {
    printf("%s not found!\n", filename);
    return false;
  }
  char line[1024];
  Material* cmat = nullptr;
  while (1) {
    s32 res = fscanf(f, "%s", line);
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
#if DEBUGOBJ
  for (s32 i = 0; i < *material_count; ++i) {
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
#endif
  return true;
}

bool
SetupMesh(Mesh* mesh, r32* verts, u32 verts_count, r32* norms, u32 norms_count)
{
  mesh->vert_count = verts_count / 3;
  mesh->norm_count = norms_count / 3;

  GLuint points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, verts_count * sizeof(GLfloat),
               &verts[0], GL_STATIC_DRAW);

  if (!points_vbo) {
    return false;
  }

  GLuint normals_vbo = 0;
  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, norms_count * sizeof(GLfloat),
               &norms[0], GL_STATIC_DRAW);
  
  if (!normals_vbo) {
    return false;
  }

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  if (!vao) {
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

  return true;
}

bool
LoadOBJ(const char* filename, Mesh* mesh)
{
  *mesh = {};
  static v3f v[kMaxVertCount];
  static u32 cv = 0;
  static v3f vn[kMaxVertCount];
  static u32 cvn = 0;

#define FREE_MEM()                             \
  memory::PopType<r32>(2 * kMaxVertCount * 8); \

  r32* verts = memory::PushType<r32>(kMaxVertCount * 8);
  u32 verts_count = 0;

  r32* norms = memory::PushType<r32>(kMaxVertCount * 8);
  u32 norms_count = 0;

  cv = 0;
  cvn = 0;

#define ADD_VERTS(vec) \
  memcpy(&verts[verts_count], &v[vec.x - 1].x, 3 * sizeof(r32));  \
  memcpy(&norms[norms_count], &vn[vec.z - 1].x, 3 * sizeof(r32)); \
  verts_count += 3;                                                  \
  norms_count += 3;

#define ADD_VERT_AND_NORM(a, b, c) \
  ADD_VERTS(a);                    \
  ADD_VERTS(b);                    \
  ADD_VERTS(c);

  FILE* f = fopen(filename, "rb");
  if (!f) {
    FREE_MEM();
    printf("%s not found!\n", filename);
    return false;
  }
  char line[1024];
  Material* mtl = nullptr;
  while (1) {
    s32 res = fscanf(f, "%s", line);
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
      for (s32 i = 0; i < mesh->material_count; ++i) {
        if (strcmp(mesh->material[i].name, mtlname) == 0) {
          mtl = &mesh->material[i];
          mtl->vert_pair[mtl->vert_pair_count] = MaterialVertPair();
          mtl->vert_pair_count = mtl->vert_pair_count + 1;
          assert(mtl->vert_pair_count < kMaxVertPair);
          break;
        }
      }
    } else if (strcmp(line, "f") == 0) {
      u32 vert_pair_idx = mtl ? mtl->vert_pair_count - 1 : 0;
      if (mtl && mtl->vert_pair[vert_pair_idx].first == u64(-1)) {
        mtl->vert_pair[vert_pair_idx].first = verts_count / 3;
      }
      s32 i = 0;
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
      if (mtl) {
        mtl->vert_pair[vert_pair_idx].count =
            (verts_count / 3) - mtl->vert_pair[vert_pair_idx].first;
      }
    }
  } 
   
  if (!SetupMesh(mesh, verts, verts_count, norms, norms_count)) {
    fclose(f);
    FREE_MEM();
    return false;
  }
  
#if DEBUGOBJ
  printf("Loaded Mesh vert count %i verts vbo %i norms vbo %i vao %i \n",
         mesh->vert_count, mesh->vert_vbo, mesh->norm_vbo, mesh->vao);

  for (s32 i = 0; i < mesh->material_count; ++i) {
    const Material* mat = &mesh->material[i];
    printf("  material %s\n", mat->name);
    for (s32 j = 0; j < mat->vert_pair_count; ++j) {
      const MaterialVertPair* vp = &mat->vert_pair[j];
      printf("    first %u count %u\n", vp->first, vp->count);
    }
  }
#endif

  fclose(f);

  FREE_MEM();
  return true;
}


}  // namespace rgg
