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
  printf("Saving map %s\n", name);
  FILE* f = fopen(name, "w+");
  if (!f) return;
  fprintf(f, "%s\n", name);
  for (s32 i = 0; i < kUsedTexture; ++i) {
    Texture* t = &kTexture[i];
    //fprintf(f, "t %s %s %i %i\n", t->texture
  }
}

}
