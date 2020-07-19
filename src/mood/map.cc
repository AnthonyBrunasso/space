#pragma once

namespace mood {

// Map File Format.
// Simple a list, in text format, of objects on the map.
//
// Texture specified with a t then followed by a list of positions that the
// texture exists at.
//
// <map_name>
// t <texture_name> <label_name> <pos>
// ...
// g <pos> <dims>
// ...

void
MapSave(const char* name)
{
  printf("Saving Map To %s\n", name);
  FILE* f = fopen(name, "w+");
  if (!f) return;
  fprintf(f, "%s\n", name);
  for (s32 i = 0; i < kUsedTexture; ++i) {
    Texture* t = &kTexture[i];
    rgg::Texture* rgg_texture = rgg::GetTexture(t->texture_id);
    fprintf(f, "t %s %s %.2f %.2f\n",
            rgg_texture->file, t->label_name, t->rect.x, t->rect.y);
  }
  fclose(f);
}

void
MapReload(const char* name)
{
  kReloadGame = true;
  strcpy(kReloadFrom, name);
}

void
MapLoadFrom(const char* name)
{
  printf("Loading Map From %s\n", kReloadFrom);
  FILE* f = fopen(name, "rb");
  if (!f) {
    printf("Unable to load map %s\n", name);
    return;
  }
  char line[32];
  fscanf(f, "%s\n", &line);
  printf("Map Name: %s\n", line);
  while (1) {
    int res = fscanf(f, "%s", line);
    if (res == EOF) break;
    if (strcmp(line, "t") == 0) {
      char texture_name[64];
      SPRITE_LABEL(label_name);
      v2f pos;
      fscanf(f, "%s %s %f %f\n",
             &texture_name, &label_name, &pos.x, &pos.y);
      animation::Sprite* sprite;
      u32 id;
      sprite = rgg::GetSprite(texture_name, &id);
      if (!sprite) {
        printf("Unable to load sprite for %s\n", texture_name);
        continue;
      }
      animation::SetLabel(label_name, sprite);
      RenderCreateTexture(
          id, Rectf(pos.x, pos.y, sprite->width, sprite->height),
          animation::Rect(sprite), label_name);
    }
  }
  fclose(f);
}

}
