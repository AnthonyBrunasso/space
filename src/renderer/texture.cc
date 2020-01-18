#pragma once

struct Texture {
  GLuint reference;
  GLenum slot;
};

struct TextureState {
  GLenum reserved_slots = GL_TEXTURE0;
};

static TextureState kTextureState;

Texture CreateTexture2D(GLenum format, uint64_t width, uint64_t height,
                        const void* data) {
  Texture texture = {};
  glGenTextures (1, &texture.reference);
  texture.slot = kTextureState.reserved_slots;
  // Texture slot now reserved.
  kTextureState.reserved_slots += 1;
  // Don't allow more than 30 textures for now.
  // TODO: Come up with a better strategy for surpassing max texture.
  assert(kTextureState.reserved_slots < GL_TEXTURE0 + 30);
  glActiveTexture(texture.slot);
  glBindTexture(GL_TEXTURE_2D, texture.reference);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    format,
    width,
    height,
    0,
    format,
    GL_UNSIGNED_BYTE,
    data
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  return texture;
}
