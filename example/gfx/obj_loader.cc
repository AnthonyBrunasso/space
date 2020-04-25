#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "math/vec.h"

constexpr int kMaxVertCount = 65538;

static v3f v[kMaxVertCount];
static uint32_t cv = 0;
static v3f vn[kMaxVertCount];
static uint32_t cvn = 0;

static float kVerts[kMaxVertCount * 10];
static uint32_t kVertElementCount;
static float kNorms[kMaxVertCount * 10];
static uint32_t kNormElementCount;

#define ADD_VERTS(vec) \
  memcpy(&kVerts[kVertElementCount], &v[vec.x - 1].x, 3 * sizeof(float));  \
  memcpy(&kNorms[kNormElementCount], &vn[vec.z - 1].x, 3 * sizeof(float)); \
  kVertElementCount += 3;                                                \
  kNormElementCount += 3;

void
AddVertAndNorm(const v3i& a, const v3i& b, const v3i& c)
{
  ADD_VERTS(a);
  ADD_VERTS(b);
  ADD_VERTS(c);
}

bool
LoadOBJ(const char* filename)
{
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
      AddVertAndNorm(first, second, third);
      second = third;
      char c = fgetc(f);
      while (c != '\n') {
        fscanf(f, " %i/%i/%i", &third.x, &third.y, &third.z);
        AddVertAndNorm(first, second, third);
        second = third;
        c = fgetc(f);
      }
    }
  } 

  printf("Loaded %s %i verts %i norms\n", filename, kVertElementCount / 3,
         kNormElementCount / 3);

  return true;
}


int
main(int argc, char** argv)
{
  LoadOBJ("example/gfx/gear.obj");

  return 0;
}
