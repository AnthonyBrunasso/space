// Usage -
//
// Run binary with 3 arguments.
//
// objtoc <objfile> <coutputfile> <typename>
//
// Example -
//
// bin\objtoc.exe cube.obj cube.cc Cube
//
// Outputs a cc file with all the vert / norm data of the file.
//
// TODO(abrunasso): This could save some space if we use index buffers in the
// renderer.


#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "math/vec.h"
#include "common/array.cc"

// NOTES...
// This file turns obj files into C arrays.
//
// Obj files contain.
//
// Vertices - Lines that start with a v.
// Vertex Texture Coords - Lines that start with a vt.
// Vertex Normals - Lines that start with a vn.
// Faces - Lines that start with an f. These lines have an arbitrary number of
// vert/texture/norm index. Therefore load all the index data first.
//
// Index data starts at 1!!!
//
// May need more precision than atof gives as it rounds results.
// atof('0.951057') = '0.95106'
// I'm ok with this for now...

struct v {
  v3f v;
};

struct vt {
  v2f v;
};

struct vn {
  v3f v;
};

constexpr int kMaxVertCount = 65538;

static float kVerts[kMaxVertCount * 3];
static int kVertCount;
static float kNorms[kMaxVertCount * 3];
static int kNormCount;

// Correspond to v, vt and vn entries in obj file.
DECLARE_ARRAY(v, kMaxVertCount);
DECLARE_ARRAY(vt, kMaxVertCount);
DECLARE_ARRAY(vn, kMaxVertCount);

void
GetAfterNextSpace(int* idx, char* line)
{
  (*idx)++;
  while (line[(*idx)++] != ' ');
}

void
FillV(char* line)
{
  if (line[0] != 'v' || line[1] != ' ') return;
  int idx = 2;
  float x = atof(&line[idx]);
  GetAfterNextSpace(&idx, line);
  float y = atof(&line[idx]);
  GetAfterNextSpace(&idx, line);
  float z = atof(&line[idx]);
  Usev()->v = v3f(x, y, z);
}

void
FillVt(char* line)
{
  if (line[0] != 'v' || line[1] != 't') return;
  int idx = 3;
  float x = atof(&line[idx]);
  GetAfterNextSpace(&idx, line);
  float y = atof(&line[idx]);
  Usevt()->v = v2f(x, y);
}

void
FillVN(char* line)
{
  if (line[0] != 'v' || line[1] != 'n') return;
  int idx = 3;
  float x = atof(&line[idx]);
  GetAfterNextSpace(&idx, line);
  float y = atof(&line[idx]);
  GetAfterNextSpace(&idx, line);
  float z = atof(&line[idx]);
  Usevn()->v = v3f(x, y, z);
}

void
GetAfterNextSlash(int* idx, char* line)
{
  (*idx)++;
  while (line[(*idx)++] != '/');
}

void
GetAfterPreviousSpace(int* idx, char* line)
{
  while (line[(*idx)] != ' ') {
    (*idx)--;
  }
  (*idx)--;
}

bool
IsNewLineNext(int idx, char* line)
{
  // If next special character is space there is more face data.
  // If next special character is new line there is not.
  while (true) {
    if (line[idx] == ' ') return false;
    if (line[idx] == '\n') return true;
    ++idx;
  }
}

void
AddTri(v3i root, v3i vi1, v3i vi2)
{
  v3f t1 = kv[root.x].v;
  v3f t2 = kv[vi1.x].v;
  v3f t3 = kv[vi2.x].v;
  kVerts[kVertCount++] = t1.x;
  kVerts[kVertCount++] = t1.y;
  kVerts[kVertCount++] = t1.z;
  kVerts[kVertCount++] = t2.x;
  kVerts[kVertCount++] = t2.y;
  kVerts[kVertCount++] = t2.z;
  kVerts[kVertCount++] = t3.x;
  kVerts[kVertCount++] = t3.y;
  kVerts[kVertCount++] = t3.z;
  v3f n1 = kvn[root.z].v;
  v3f n2 = kvn[vi1.z].v;
  v3f n3 = kvn[vi2.z].v;
  kNorms[kNormCount++] = n1.x;
  kNorms[kNormCount++] = n1.y;
  kNorms[kNormCount++] = n1.z;
  kNorms[kNormCount++] = n2.x;
  kNorms[kNormCount++] = n2.y;
  kNorms[kNormCount++] = n2.z;
  kNorms[kNormCount++] = n3.x;
  kNorms[kNormCount++] = n3.y;
  kNorms[kNormCount++] = n3.z;
  printf("%i\n", kVertCount);
}

void
FillF(char* line)
{
  if (line[0] != 'f' || line[1] != ' ') return;

  printf(line);

  int idx = 2;
  // Get first  
  v3i root;
  // Indices in obj files start at 1 so subtract 1 here to be consistent with C
  root.x = atoi(&line[idx]) - 1;
  GetAfterNextSlash(&idx, line);
  root.y = atoi(&line[idx]) - 1;
  GetAfterNextSlash(&idx, line);
  root.z = atoi(&line[idx]) - 1;
  while (line[idx] != '\n') {
    printf("%i\n", idx);
    GetAfterNextSpace(&idx, line);
    v3i vertidx1, vertidx2;
    vertidx1.x = atoi(&line[idx]) - 1;
    GetAfterNextSlash(&idx, line);
    vertidx1.y = atoi(&line[idx]) - 1;
    GetAfterNextSlash(&idx, line);
    vertidx1.z = atoi(&line[idx]) - 1;

    GetAfterNextSpace(&idx, line);

    vertidx2.x = atoi(&line[idx]) - 1;
    GetAfterNextSlash(&idx, line);
    vertidx2.y = atoi(&line[idx]) - 1;
    GetAfterNextSlash(&idx, line);
    vertidx2.z = atoi(&line[idx]) - 1;

    // Add this triangle and norm to our tri list.
    AddTri(root, vertidx1, vertidx2);

    if (IsNewLineNext(idx, line)) break;

    GetAfterPreviousSpace(&idx, line);
  }
  //printf("root(%i,%i,%i)\n", root.x, root.y, root.z);
}

int
main(int argc, char** argv)
{
  // Requires file.
  assert(argc > 1);

  printf("Converting %s to C...\n", argv[1]);

  FILE* fptr;
  uint8_t* buffer;
  uint32_t file_length;
  fptr = fopen(argv[1], "rb");
  char line[16384];  // Should be big enough!.

  
  while (fgets(line, sizeof(line), fptr)) {
    printf("V\n");
    FillV(line);
    printf("Vt\n");
    FillVt(line);
    printf("VN\n");
    FillVN(line);

    printf("F\n");
    FillF(line);
  }

  printf("Verts...\n");
  for (int i = 0; i < kVertCount; i += 3) {
    printf("%.3f,%.3f,%.3f\n", kVerts[i], kVerts[i + 1], kVerts[i + 2]);
  }

  printf("Norms...\n");
  for (int i = 0; i < kNormCount; i += 3) {
    printf("%.3f,%.3f,%.3f\n", kNorms[i], kNorms[i + 1], kNorms[i + 2]);
  }

  FILE* fout;

  char* fn = argv[2];
  char* n = argv[3];
  fout = fopen(fn, "w");
  fprintf(fout, "#pragma once\n\n");
  fprintf(fout, "namespace rgg {\n\n");
  fprintf(fout, "constexpr int k%sVertCount = %i;\n\n", n, kVertCount / 3);
  fprintf(fout, "static float k%sVerts[%i] = {\n", n, kVertCount);
  for (int i = 0; i < kVertCount; i += 3) {
    fprintf(fout, "  %.3f,%.3f,%.3f,\n", kVerts[i], kVerts[i + 1], kVerts[i + 2]);
  }
  fprintf(fout, "};\n\n");

  fprintf(fout, "static float k%sVertNorms[%i] = {\n", n, kNormCount);
  for (int i = 0; i < kNormCount; i += 3) {
    fprintf(fout, "  %.3f,%.3f,%.3f,\n", kNorms[i], kNorms[i + 1], kNorms[i + 2]);
  }
  fprintf(fout, "};\n\n");

  fprintf(fout, "}  // namespace rgg");

  fclose(fout);

  return 0;
}
