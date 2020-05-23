#pragma once

#include "math/rect.h"

namespace animation
{

static constexpr uint32_t kMaxLabelSize = 16;
static constexpr uint32_t kMaxCoordSize = 16;

struct Label {
  char name[64];
  v2i coord[kMaxCoordSize];
  uint32_t coord_size = 0;
  // Number of frames to wait before traversing to next image.
  uint32_t frame_count = 0;
};

struct Sprite {
  // Texture specific data.
  char texture_name[64];
  // Width and height of sprite within the texture.
  uint32_t width = 0;
  uint32_t height = 0;
  Label label[kMaxLabelSize];
  uint32_t label_size = 0;

  // Animation specific data.
  uint32_t last_update = 0;
  uint32_t label_idx = 0;
  uint32_t label_coord_idx = 0;

  bool IsValid() const { return width && height; }
};

bool
LoadAnimation(const char* filename, Sprite* sprite)
{
  FILE* f = fopen(filename, "rb");
  if (!f) return false;
  *sprite = {};
  char line[64];
  Label* clabel = nullptr;
  while (1) {
    int res = fscanf(f, "%s", line);
    if (res == EOF) break;
    if (strcmp(line, "t") == 0) {
      char tex_name[64] = {};
      fscanf(f, "%s\n", tex_name);
      strcpy(sprite->texture_name, tex_name);
    } else if (strcmp(line, "w") == 0) {
      fscanf(f, "%u\n", &sprite->width);
    } else if (strcmp(line, "h") == 0) {
      fscanf(f, "%u\n", &sprite->height);
    } else if (strcmp(line, "l") == 0) {
      char label_name[64] = {};
      fscanf(f, "%s\n", label_name);
      assert(sprite->label_size < kMaxLabelSize);
      clabel = &sprite->label[sprite->label_size++];
      strcpy(clabel->name, label_name);
    } else if (strcmp(line, "c") == 0) {
      assert(clabel);
      assert(clabel->coord_size + 1 < kMaxLabelSize);
      uint32_t i = clabel->coord_size;
      fscanf(f, "%u %u\n", &clabel->coord[i].x, &clabel->coord[i].y);
      ++clabel->coord_size;
    } else if (strcmp(line, "f") == 0) {
      assert(clabel);
      fscanf(f, "%u\n", &clabel->frame_count);
    } else { continue; }  // Unrecognized line
  }
  fclose(f);
  printf("Loaded anim file %s\n", filename);
  printf("  texture %s\n", sprite->texture_name);
  printf("  width x height %ux%u\n", sprite->width, sprite->height);
  for (int i = 0; i < sprite->label_size; ++i) {
    Label* label = &sprite->label[i];
    printf("  label %s\n", label->name);
    for (int j = 0; j < label->coord_size; ++j) {
      printf("    %u %u\n", label->coord[j].x, label->coord[j].y);
    }
    printf("    frames %u\n", label->frame_count);
  }
  return true;
}

void
SetLabel(const char* label, Sprite* sprite)
{
  for (int i = 0; i < sprite->label_size; ++i) {
    Label* l = &sprite->label[i];
    if (strcmp(l->name, label) == 0) {
      sprite->last_update = 0;
      sprite->label_idx = i;
      sprite->label_coord_idx = 0;
      return;
    }
  }
}

Rectf
Update(Sprite* sprite)
{
  sprite->last_update++;
  Label* l = &sprite->label[sprite->label_idx];
  if (sprite->last_update > l->frame_count) {
    sprite->label_coord_idx += 1;
    if (sprite->label_coord_idx >= l->coord_size) sprite->label_coord_idx = 0;
    sprite->last_update = 0;
  }
  v2i c = l->coord[sprite->label_coord_idx];
  return Rectf(sprite->width * c.y, sprite->height * c.x,
               sprite->width, sprite->height);
}

}
