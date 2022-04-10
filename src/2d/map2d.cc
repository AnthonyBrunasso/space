#pragma once

#include "map.pb.h"

// TODO: This is all copy pasta from editor_render_target, seems like maybe should generify it.
struct Layer2dSurface {
  bool IsValid() const { return render_target.IsValid(); }
  r32 width() const { return render_target.width(); }
  r32 height() const { return render_target.height(); }
  v2f Dims() const { return v2f( width(), height() ); }
  Rectf rect() const { return Rectf(v2f(0.f, 0.f), Dims()); };
  const rgg::Texture& texture() const { return render_target.texture; }
  rgg::Camera camera;
  rgg::Surface render_target;
};

Layer2dSurface CreateLayer2dWithTexture(v2f dims, const rgg::Texture& texture) {
  Layer2dSurface surface;
  surface.camera.position = v3f(0.f, 0.f, 0.f);
  surface.camera.dir = v3f(0.f, 0.f, -1.f);
  surface.camera.up = v3f(0.f, 1.f, 0.f);
  surface.camera.viewport = dims;
  surface.render_target = rgg::CreateSurfaceFromTexture(texture);
  return surface;
}

Layer2dSurface CreateLayer2dSurface(v2f dims) {
  Layer2dSurface surface;
  surface.camera.position = v3f(0.f, 0.f, 0.f);
  surface.camera.dir = v3f(0.f, 0.f, -1.f);
  surface.camera.up = v3f(0.f, 1.f, 0.f);
  surface.camera.viewport = dims;
  surface.render_target = rgg::CreateSurface(GL_RGBA, (u64)dims.x, (u64)dims.y);
  rgg::BeginRenderTo(surface.render_target);
  // Without this we have no alpha.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.f, 0.f, 0.f, 0.f);
  return surface;
}

void DestroyLayer2dSurface(Layer2dSurface* surface) {
  if (surface->IsValid()) {
    rgg::DestroySurface(&surface->render_target);
  }
  *surface = {};
}

class RenderToLayer2dSurface {
public:
  RenderToLayer2dSurface(const Layer2dSurface& surface) : mod_observer_(surface.camera) {
    rgg::BeginRenderTo(surface.render_target);
  }
  ~RenderToLayer2dSurface() {
    rgg::EndRenderTo();
  }

  rgg::ModifyObserver mod_observer_;
};

class Layer2d {
public:
  // Construct surface.
  void Initialize(const Rectf& world_rect, v4f color = v4f(0.f, 0.f, 0.f, 0.f));
  void InitializeWithTexture(const rgg::Texture& texture);
  void AddTexture(const rgg::Texture* texture, const Rectf& src_rect, const Rectf& dest_rect);

  void Render(r32 scale = 1.f);

  bool IsSurfaceValid() const { return surface_.IsValid(); }

  v4f background_color() const;
  const rgg::Surface& GetSurface() const;
  const rgg::Texture& GetTexture() const;

  v2f Dims() const { return surface_.Dims(); }
  
  Layer2dSurface surface_;
  v4f background_color_;
  Rectf world_rect_;
};

class Map2d {
public:
  Map2d() = default;

  static Map2d LoadFromProto(const proto::Map2d& proto);
  static bool LoadFromProtoFile(const char* filename, Map2d* map);

  void AddLayer(const Rectf& world_rect);
  void AddTexture(s32 layer_idx, const rgg::Texture* texture, const Rectf& src_rect, const Rectf& dest_rect);
  void Render(r32 scale = 1.f);

  const rgg::Surface& GetSurface(s32 layer_idx) const;
  // Gets layer_idx's rendering texture
  const rgg::Texture& GetTexture(s32 layer_idx);
  const Layer2d& GetLayer(s32 layer_idx) const;
  // Need a map name to generate layer asset path names.
  proto::Map2d ToProto(const char* map_name) const;

  bool HasLayers() const { return !layers_.empty(); }
  s32 GetLayerCount() const { return layers_.size(); }

  std::vector<Layer2d> layers_;
};

void Layer2d::Initialize(const Rectf& world_rect, v4f color) {
  world_rect_ = world_rect;
  if (IsSurfaceValid()) DestroyLayer2dSurface(&surface_);
  surface_ = CreateLayer2dSurface(world_rect.Dims());
  LOG(INFO, "Creating layer texture %u with dims %.2f %.2f", surface_.texture().reference, world_rect.Dims().x, world_rect.Dims().y);
  RenderToLayer2dSurface render_to(surface_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glClearColor(0.f, 0.f, 0.f, 0.f);
  background_color_ = color;
  rgg::RenderRectangle(world_rect_, background_color());
}

void Layer2d::InitializeWithTexture(const rgg::Texture& texture) {
  world_rect_.x = texture.width / -2.f;
  world_rect_.y = texture.height / -2.f;
  world_rect_.width = texture.width;
  world_rect_.height = texture.height;
  surface_ = CreateLayer2dWithTexture(world_rect_.Dims(), texture);
}

void Layer2d::AddTexture(const rgg::Texture* texture, const Rectf& src_rect, const Rectf& dest_rect) {
  if (!IsSurfaceValid()) {
    LOG(WARN, "Trying to render to a Layer2d that has not been intialized.");
  }
  RenderToLayer2dSurface render_to(surface_);
  rgg::RenderTexture(*texture, src_rect, dest_rect);
}

void Layer2d::Render(r32 scale) {
  assert(IsSurfaceValid());
  Rectf dest = Rectf(surface_.Dims() / -2.f, surface_.Dims());
  if (scale != 1.f) {
    dest.x *= scale;
    dest.y *= scale;
    dest.width *= scale;
    dest.height *= scale;
  }
  rgg::RenderTexture(surface_.render_target.texture, surface_.rect(), dest);
}

v4f Layer2d::background_color() const {
  // Use imgui's default if ours is unspecified.
  if (background_color_ == v4f(0.f, 0.f, 0.f, 0.f)) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 imcolor = style.Colors[ImGuiCol_WindowBg];
    return v4f(imcolor.x, imcolor.y, imcolor.z, 0.f);
  }
  return background_color_;
}

const rgg::Surface& Layer2d::GetSurface() const {
  return surface_.render_target;
}

const rgg::Texture& Layer2d::GetTexture() const {
  return surface_.texture();
}

Map2d Map2d::LoadFromProto(const proto::Map2d& proto) {
  Map2d map;
  rgg::TextureInfo texture_info;
  texture_info.min_filter = GL_NEAREST_MIPMAP_NEAREST;
  texture_info.mag_filter = GL_NEAREST;
  for (const proto::Layer2d proto_layer : proto.layers()) {
    rgg::Texture texture;
    if (!rgg::LoadFromFile(proto_layer.image_file().c_str(), texture_info, &texture)) {
      LOG(ERR, "Cannot load texture file %s... This is probably bad. Check that it exists?",
          proto_layer.image_file().c_str());
      continue;
    }
    Layer2d layer;
    // TODO: world_rect should be loaded from file.
    layer.InitializeWithTexture(texture);
    map.layers_.push_back(std::move(layer));
  }
  return map;
}

bool Map2d::LoadFromProtoFile(const char* filename, Map2d* map) {
  proto::Map2d proto;
  std::fstream inp(filename, std::ios::in | std::ios::binary);
  if (!proto.ParseFromIstream(&inp)) {
    return false;
  }
  *map = LoadFromProto(proto);
  return true;
}

void Map2d::AddLayer(const Rectf& world_rect) {
  Layer2d layer;
  // TODO: AddLayer should pass in bounds.
  layer.Initialize(world_rect);
  layers_.push_back(std::move(layer));
}

void Map2d::AddTexture(s32 layer_idx, const rgg::Texture* texture, const Rectf& src_rect, const Rectf& dest_rect) {
  assert(layer_idx < layers_.size());
  Layer2d* layer = &layers_[layer_idx];
  layer->AddTexture(texture, src_rect, dest_rect);
}

const rgg::Surface& Map2d::GetSurface(s32 layer_idx) const {
  assert(layer_idx < layers_.size());
  return layers_[layer_idx].GetSurface();
}

// Gets layer_idx's rendering texture
const rgg::Texture& Map2d::GetTexture(s32 layer_idx) {
  assert(layer_idx < layers_.size());
  return layers_[layer_idx].GetTexture();
}

const Layer2d& Map2d::GetLayer(s32 layer_idx) const {
  assert(layer_idx < layers_.size());
  return layers_[layer_idx];
}

// NOTE: THIS MAKES DIRECTORIES IN ASSET/... Assumption is the caller saves the ncessary layer data.
proto::Map2d Map2d::ToProto(const char* map_name) const {
  assert(strlen(map_name) > 0);
  proto::Map2d map;
  s32 i = 0;
  for (const Layer2d& layer : layers_) {
    proto::Layer2d* proto_layer = map.add_layers();
    std::string image_file("./asset/maps/" + std::string(map_name) + "/");
    // Make the directory for the map in asset/{map_name}. Also we save all paths in linuxy format,
    // with forward slashes. But the path must be sanitized to actually make the directory.
    filesystem::MakeDirectory(filesystem::SanitizePath(image_file).c_str());
    // TODO: Is it ok to assume all layers get saved out as pngs?
    image_file += "layer_" + std::to_string(i++) + ".png";
    proto_layer->set_image_file(image_file);
    proto_layer->set_width(layer.Dims().x);
    proto_layer->set_height(layer.Dims().y);
  }
  return map;
}

void Map2d::Render(r32 scale) {
  for (s32 i = 0; i < layers_.size(); ++i) {
    layers_[i].Render(scale);
  }
}
