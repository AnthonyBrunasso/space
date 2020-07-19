#pragma once

#include "math/rect.h"

namespace animation
{

static constexpr u32 kMaxLabelSize = 16;
static constexpr u32 kMaxCoordSize = 16;

#define SPRITE_LABEL(name) char name[32];

struct Label {
  SPRITE_LABEL(name);

  v2i coord[kMaxCoordSize];
  u32 coord_size = 0;
  // Number of frames to wait before traversing to next image.
  u32 frame_count = 0;
};

struct Sprite {
  // Texture specific data.
  char texture_name[64];
  // Width and height of sprite within the texture.
  u32 width = 0;
  u32 height = 0;
  Label label[kMaxLabelSize];
  u32 label_size = 0;

  // Animation specific data.
  u32 last_update = 0;
  u32 label_idx = 0;
  u32 label_coord_idx = 0;
  b8 mirror = false;

  b8 IsValid() const { return width && height; }
};

b8
LoadAnimation(const char* filename, Sprite* sprite)
{
  // Sprite already loaded.
  if (sprite->IsValid()) return true;
  FILE* f = fopen(filename, "rb");
  if (!f) return false;
  *sprite = {};
  char line[64];
  Label* clabel = nullptr;
  while (1) {
    s32 res = fscanf(f, "%s", line);
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
      char label_name[8] = {};
      fscanf(f, "%s\n", label_name);
      assert(sprite->label_size < kMaxLabelSize);
      clabel = &sprite->label[sprite->label_size++];
      strcpy(clabel->name, label_name);
    } else if (strcmp(line, "c") == 0) {
      assert(clabel);
      assert(clabel->coord_size + 1 < kMaxLabelSize);
      u32 i = clabel->coord_size;
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
  for (s32 i = 0; i < sprite->label_size; ++i) {
    Label* label = &sprite->label[i];
    printf("  label %s\n", label->name);
    for (s32 j = 0; j < label->coord_size; ++j) {
      printf("    %u %u\n", label->coord[j].x, label->coord[j].y);
    }
    printf("    frames %u\n", label->frame_count);
  }
  return true;
}

void
SetLabel(const char* label, Sprite* sprite, bool mirror = false)
{
  for (s32 i = 0; i < sprite->label_size; ++i) {
    Label* l = &sprite->label[i];
    if (strcmp(l->name, label) == 0) {
      if (sprite->label_idx == i) return;
      sprite->last_update = 0;
      sprite->label_idx = i;
      sprite->label_coord_idx = 0;
      sprite->mirror = mirror;
      return;
    }
  }
}

Rectf
Update(Sprite* sprite, u32* anim_frame)
{
  sprite->last_update++;
  Label* l = &sprite->label[sprite->label_idx];
  if (sprite->last_update > l->frame_count) {
    *anim_frame += 1;
    if (*anim_frame >= l->coord_size) *anim_frame = 0;
    sprite->last_update = 0;
  }
  v2i c = l->coord[*anim_frame];
  return Rectf(sprite->width * c.y, sprite->height * c.x,
               sprite->width, sprite->height);
}

Rectf
Update(Sprite* sprite)
{
  return Update(sprite, &sprite->label_coord_idx);
}

Rectf
Rect(Sprite* sprite)
{
  Label* l = &sprite->label[sprite->label_idx];
  v2i c = l->coord[sprite->label_coord_idx];
  return Rectf(sprite->width * c.y, sprite->height * c.x,
               sprite->width, sprite->height);

}

}
