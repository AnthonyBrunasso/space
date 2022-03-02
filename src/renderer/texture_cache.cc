#pragma once

typedef u32 TextureId;

struct TextureHandle {
  TextureId id = 0;
  Texture texture;
};

struct TextureFileToId {
  TextureId id = 0;
};

#define RGG_TEXTURE_MAX 64

DECLARE_HASH_ARRAY(TextureHandle, RGG_TEXTURE_MAX);
DECLARE_HASH_MAP_STR(TextureFileToId, RGG_TEXTURE_MAX);

TextureId LoadTexture(const char* texture_file, const TextureInfo& texture_info) {
  u32 len = strlen(texture_file);
  TextureFileToId* loaded_file_to_id = FindTextureFileToId(texture_file, len);
  if (loaded_file_to_id) return loaded_file_to_id->id;
  assert(kUsedTextureHandle < RGG_TEXTURE_MAX);
  TextureHandle* t = UseTextureHandle();
  if (!LoadFromFile(texture_file, texture_info, &t->texture)) {
    return 0;
  }
  TextureFileToId* file_to_id = UseTextureFileToId(texture_file, len);
  file_to_id->id = t->id;
  return t->id; 
}

const Texture* GetTexture(TextureId id) {
  TextureHandle* handle = FindTextureHandle(id);
  if (!handle) return nullptr;
  return &handle->texture;
}

const Texture* GetTexture(const char* file) {
  TextureFileToId* file_to_id = FindTextureFileToId(file, strlen(file));
  if (!file_to_id) return nullptr;
  return GetTexture(file_to_id->id);
}

void RenderTexture(TextureId id, const Rectf& src, const Rectf& dest, bool mirror = false) {
  const Texture* t = GetTexture(id);
  if (!t) return;
  RenderTexture(*t, src, dest, mirror);
}

void IterateTextures(const std::function<void(const rgg::TextureHandle*)> func) {
  for (s32 i = 0; i < kUsedTextureHandle; ++i) {
    func(&kTextureHandle[i]);
  }
}
