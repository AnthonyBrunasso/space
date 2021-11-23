// namespace live {

enum class TerrainAsset {
  kAll = 0, // The whole spritesheet, mostly for debugging.
  kTree = 1,
  kGrassBottomLeft = 2,
  kGrassBottomMiddle = 3,
  kGrassBottomRight = 4,
  kGrassMiddleLeft = 5,
  kGrassMiddleMiddle = 6,
  kGrassMiddleRight = 7,
};

constexpr r32 kTW = 16;
constexpr r32 kTH = 16;

static Rectf kTreeRect = Rectf(143.f, 32.5f, kTW, kTH);

static Rectf kGrassBottomLeftRect = Rectf(0.f, 41.f, kTW, kTH);
static Rectf kGrassBottomMiddleRect = Rectf(16.f, 41.f, kTW, kTH);
static Rectf kGrassBottomRightRect = Rectf(32.f, 41.f, kTW, kTH);

static Rectf kGrassMiddleLeftRect = Rectf(0.f, 25.f, kTW, kTH);
static Rectf kGrassMiddleMiddleRect = Rectf(16.f, 25.f, kTW, kTH);
static Rectf kGrassMiddleRightRect = Rectf(32.f, 25.f, kTW, kTH);


struct AssetStorage {
  u32 terrain_texture_id;
};

static AssetStorage kAssets;

void
AssetLoadAll()
{
  // terrain is 256 by 256
  // rows: 16 cols: 16
  kAssets.terrain_texture_id = rgg::LoadTexture("asset/terrain.tga", rgg::DefaultTextureInfo());
  assert(kAssets.terrain_texture_id != 0);
}

void
AssetTerrainRender(v2f pos = v2f(0.f, 0.f), TerrainAsset asset = TerrainAsset::kAll)
{
  rgg::Texture* texture = rgg::GetTexture(kAssets.terrain_texture_id);
  //printf("%.2f %.2f\n", texture->width, texture->height);
  //exit(0);
  switch (asset) {
    case TerrainAsset::kAll: {
      Rectf rect(pos.x, pos.y, texture->width, texture->height);
      rgg::RenderTexture(*texture, rect, rect);
    } break;
    case TerrainAsset::kTree: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kTreeRect, dest);
    } break;
    case TerrainAsset::kGrassBottomLeft: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kGrassBottomLeftRect, dest);
    } break;
    case TerrainAsset::kGrassBottomMiddle: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kGrassBottomMiddleRect, dest);
    } break;
    case TerrainAsset::kGrassBottomRight: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kGrassBottomRightRect, dest);
    } break;
    case TerrainAsset::kGrassMiddleLeft: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kGrassMiddleLeftRect, dest);
    } break;
    case TerrainAsset::kGrassMiddleMiddle: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kGrassMiddleMiddleRect, dest);
    } break;
    case TerrainAsset::kGrassMiddleRight: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kGrassMiddleRightRect, dest);
    } break;
    default:
      assert(!"Implement rendering for this asset.");
      break;
  }
}

// }
