#pragma once

struct TextureHandle {
  u32 id = 0;
  Texture texture;
  animation::Sprite sprite;
};

struct TextureFileToId {
  u32 id = 0;
};

#define TEXTURE_MAX 16

DECLARE_HASH_ARRAY(TextureHandle, TEXTURE_MAX);
DECLARE_HASH_MAP_STR(TextureFileToId, TEXTURE_MAX);

u32
LoadTextureAndSprite(const char* texture_file, const char* sprite_file,
                     const TextureInfo& texture_info)
{
  u32 len = strlen(texture_file);
  TextureFileToId* loaded_file_to_id = FindTextureFileToId(texture_file, len);
  if (loaded_file_to_id) return loaded_file_to_id->id;
  assert(kUsedTextureHandle < TEXTURE_MAX);
  TextureHandle* t = UseTextureHandle();
  if (!LoadTGA(texture_file, texture_info, &t->texture)) {
    printf("Unable to load %s\n", texture_file);
    return 0;
  }
  if (!LoadAnimation(sprite_file, &t->sprite)) {
    printf("Unable to load %s\n", sprite_file);
    return 0;
  }
  TextureFileToId* file_to_id = UseTextureFileToId(texture_file, len);
  file_to_id->id = t->id;
  return t->id; 
}

Texture*
GetTexture(u32 id)
{
  TextureHandle* handle = FindTextureHandle(id);
  if (!handle) return nullptr;
  return &handle->texture;
}

Texture*
GetTexture(const char* file)
{
  TextureFileToId* file_to_id = FindTextureFileToId(file, strlen(file));
  if (!file_to_id) return nullptr;
  return GetTexture(file_to_id->id);
}

animation::Sprite*
GetSprite(u32 id)
{
  TextureHandle* handle = FindTextureHandle(id);
  if (!handle) return nullptr;
  return &handle->sprite;
}

animation::Sprite*
GetSprite(const char* file, u32* id = nullptr)
{
  TextureFileToId* file_to_id = FindTextureFileToId(file, strlen(file));
  if (!file_to_id) return nullptr;
  if (id) *id = file_to_id->id;
  return GetSprite(file_to_id->id);
}

bool
GetTextureAndSprite(u32 id, Texture* texture, animation::Sprite* sprite)
{
  TextureHandle* handle = FindTextureHandle(id);
  if (!handle) return false;
  texture = &handle->texture;
  sprite = &handle->sprite;
  return true;
}

bool
GetTextureAndSprite(const char* file, Texture* texture, animation::Sprite* sprite,
                    u32* id = nullptr)
{
  TextureFileToId* file_to_id = FindTextureFileToId(file, strlen(file));
  if (!file_to_id) return false;
  if (id) *id = file_to_id->id;
  return GetTextureAndSprite(file_to_id->id, texture, sprite);
}

void
RenderTexture(u32 id, const Rectf& src, const Rectf& dest, bool mirror = false)
{
  Texture* t = GetTexture(id);
  if (!t) return;
  RenderTexture(*t, src, dest, mirror);
}
