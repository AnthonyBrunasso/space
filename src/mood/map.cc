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
// g <pos> <dims> <flags> <user_flags>
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
  for (u32 i = 0; i < physics::kUsedParticle2d; ++i) {
    physics::Particle2d* p = &physics::kParticle2d[i];
    if (FLAGGED(p->user_flags, kParticleCollider)) {
      fprintf(f, "g %.2f %2f %.2f %.2f %.2f %u %u\n",
              p->position.x, p->position.y, p->dims.x, p->dims.y,
              p->inverse_mass, p->flags, p->user_flags);
    }
  }
  FOR_EACH_ENTITY_P(Spawner, s, p, {
    fprintf(f, "s %.2f %.2f %u\n", p->position.x, p->position.y,
            s->spawner_type);
  });
  FOR_EACH_ENTITY_P(Obstacle, o, p, {
    fprintf(f, "o %.2f %.2f %.2f %.2f %u\n",
            p->position.x, p->position.y, p->dims.x, p->dims.y,
            o->obstacle_type);
  });
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
    } else if (strcmp(line, "g") == 0) {
      v2f pos;
      v2f dims;
      r32 inv_mass = 0.f;
      u32 flags;
      u32 user_flags;
      fscanf(f, "%f %f %f %f %f %u %u\n", &pos.x, &pos.y, &dims.x, &dims.y,
             &inv_mass, &flags, &user_flags);
      physics::Particle2d* p = physics::CreateParticle2d(pos, dims);
      p->inverse_mass = inv_mass;
      p->flags = flags;
      p->user_flags = user_flags;
    } else if (strcmp(line, "s") == 0) {
      v2f pos;
      SpawnerType type;
      fscanf(f, "%f %f %u\n", &pos.x, &pos.y, &type);
      SpawnerCreate(pos, type);
    } else if (strcmp(line, "o") == 0) {
      v2f pos, dims;
      ObstacleType type;
      fscanf(f, "%f %f %f %f %u\n", &pos.x, &pos.y, &dims.x, &dims.y, &type);
      ObstacleCreate(pos, dims, type);
    }
  }
  fclose(f);
}

}
