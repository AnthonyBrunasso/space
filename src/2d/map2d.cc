#pragma once

class Layer2d {
public:
  struct Texture {
    rgg::TextureId texture_id;
    // Source rect in texture space
    Rectf src_rect;
    // Destination rect in world space
    Rectf dest_rect;
  };

  void Render(r32 scale = 1.f);

  std::vector<Texture> textures_;
};

class Map2d {
public:
  Map2d() = default;
  Map2d(v2f dims, s32 layers_size = 1);

  void AddLayer();
  void AddTexture(s32 layer_idx, Layer2d::Texture texture);
  void Render(r32 scale = 1.f);

  std::vector<Layer2d> layers_;
  // Min and max bounds of the world. Origin in the center of this rect and should correspond with 0,0.
  Rectf world_rect_;
};

void Layer2d::Render(r32 scale) {
  for (const Texture& texture : textures_) {
    const rgg::Texture* rgg_texture = rgg::GetTexture(texture.texture_id);
    if (!rgg_texture) {
      LOG(WARN, "Couldn't render texture");
    } else {
      if (scale != 1.f) {
        Rectf dest = texture.dest_rect;
        dest.x *= scale;
        dest.y *= scale;
        dest.width *= scale;
        dest.height *= scale;
        rgg::RenderTexture(*rgg_texture, texture.src_rect, dest);
      } else {
        rgg::RenderTexture(*rgg_texture, texture.src_rect, texture.dest_rect);
      }
    }
  }
}

Map2d::Map2d(v2f dims, s32 layers_size) {
  assert(dims.x > 0.f && dims.y > 0.f);
  world_rect_.x = -dims.x / 2.f;
  world_rect_.y = -dims.y / 2.f;
  world_rect_.width = dims.x;
  world_rect_.height = dims.y;
  for (s32 i = 0; i < layers_size; ++i) AddLayer();
}

void Map2d::AddLayer() {
  layers_.push_back({});
}

void Map2d::AddTexture(s32 layer_idx, Layer2d::Texture texture) {
  assert(layer_idx < layers_.size());
  layers_[layer_idx].textures_.push_back(texture);
}

void Map2d::Render(r32 scale) {
  for (s32 i = 0; i < layers_.size(); ++i) {
    layers_[i].Render(scale);
  }
}
