#pragma once

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

Layer2dSurface CreateLayer2dSurface(v2f dims) {
  Layer2dSurface surface;
  surface.camera.position = v3f(0.f, 0.f, 0.f);
  surface.camera.dir = v3f(0.f, 0.f, -1.f);
  surface.camera.up = v3f(0.f, 1.f, 0.f);
  surface.camera.viewport = dims;
  surface.render_target = rgg::CreateSurface(GL_RGBA, (u64)dims.x, (u64)dims.y);
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
    // Without this we have no alpha.
    glClearColor(0.f, 0.f, 0.f, 0.f);
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
  void AddTexture(const rgg::Texture* texture, const Rectf& src_rect, const Rectf& dest_rect);

  void Render(r32 scale = 1.f);

  bool IsSurfaceValid() const { return surface_.IsValid(); }

  v4f background_color() const;
  const rgg::Texture& GetTexture() const;
  
  Layer2dSurface surface_;
  v4f background_color_;
  Rectf world_rect_;
};

class Map2d {
public:
  Map2d() = default;
  Map2d(v2f dims, s32 layers_size = 1);

  void AddLayer();
  void AddTexture(s32 layer_idx, const rgg::Texture* texture, const Rectf& src_rect, const Rectf& dest_rect);
  void Render(r32 scale = 1.f);

  // Gets layer_idx's rendering texture
  const rgg::Texture& GetTexture(s32 layer_idx);

  bool HasLayers() const { return !layers_.empty(); }
  s32 GetLayerCount() const { return layers_.size(); }

  std::vector<Layer2d> layers_;
  // Min and max bounds of the world. Origin in the center of this rect and should correspond with 0,0.
  Rectf world_rect_;
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
  // TODO:Flipping this needs some investigation. But I think it's likely due to how assets loaded from 
  // disk look differently than how rendering assumes. I think probably I need to load assets non flipped
  // and modify the RenderTexture call uv ordering.
  /*LOG(INFO, "dest %.2f %.2f %.2f %.2f",
      dest.x,
      dest.y,
      dest.width,
      dest.height);*/
  rgg::RenderTexture(surface_.render_target.texture, surface_.rect(), dest, false, true);
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

const rgg::Texture& Layer2d::GetTexture() const {
  return surface_.texture();
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
  Layer2d layer;
  layer.Initialize(world_rect_);
  layers_.push_back(std::move(layer));
}

void Map2d::AddTexture(s32 layer_idx, const rgg::Texture* texture, const Rectf& src_rect, const Rectf& dest_rect) {
  assert(layer_idx < layers_.size());
  Layer2d* layer = &layers_[layer_idx];
  layer->AddTexture(texture, src_rect, dest_rect);
}

// Gets layer_idx's rendering texture
const rgg::Texture& Map2d::GetTexture(s32 layer_idx) {
  assert(layer_idx < layers_.size());
  return layers_[layer_idx].GetTexture();
}



void Map2d::Render(r32 scale) {
  for (s32 i = 0; i < layers_.size(); ++i) {
    layers_[i].Render(scale);
  }
}
