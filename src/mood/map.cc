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
  LOG(INFO, "Saving Map To %s", name);
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
  ECS_ITR1(spawner_itr, kSpawnerComponent);
  while (spawner_itr.Next()) {
    SpawnerComponent* s = spawner_itr.c.spawner;
    physics::Particle2d* p = ecs::GetParticle(spawner_itr.e);
    fprintf(f, "s %.2f %.2f %u\n", p->position.x, p->position.y,
            s->spawner_type);
  }
  ECS_ITR1(obstacle_itr, kObstacleComponent);
  while (obstacle_itr.Next()) {
    ObstacleComponent* o = obstacle_itr.c.obstacle;
    physics::Particle2d* p = ecs::GetParticle(obstacle_itr.e);
    fprintf(f, "o %.2f %.2f %.2f %.2f %u\n",
            p->position.x, p->position.y, p->dims.x, p->dims.y,
            o->obstacle_type);
  }
  fclose(f);
}

void
MapCreateEmpty(const char* name)
{
  FILE* f = fopen(name, "w+");
  if (!f) return;
  LOG(INFO, "Map create empty %s", name);
  fprintf(f, "%s\n", name);
  fclose(f);
}

void
MapReload(const char* name)
{
  kReloadGame = true;
  strcpy(kCurrentMapName, name);
}

void
MapLoadFrom(const char* name)
{
  LOG(INFO, "Loading Map From %s", kCurrentMapName);
  FILE* f = fopen(name, "rb");
  if (!f) {
    LOG(WARN, "Unable to load map %s", name);
    return;
  }
  char line[32];
  fscanf(f, "%s\n", &line);
  LOG(INFO, "Map Name: %s", line);
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
        LOG(WARN, "Unable to load sprite for %s", texture_name);
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

void
MapUniqueNameFinder(const char* filename)
{
  if (strncmp(filename, "asset/level", 11) == 0) {
    uint32_t lvl_num;
    sscanf(filename, "asset/level_%u.map", &lvl_num);
    if (lvl_num >= kMapNum) {
      ++kMapNum;
    }
  }
  strcpy(kCurrentMapName, "asset/level_");
  char num[8];
  sprintf(num, "%d", kMapNum);
  strcat(kCurrentMapName, num);
  strcat(kCurrentMapName, ".map");
}

void
MapGenerateUniqueName()
{
  //filesystem::WalkDirectory("asset/", MapUniqueNameFinder);
}

}
