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
  kDirtBottomLeft = 8,
  kDirtBottomMiddle = 9,
  kDirtBottomRight = 10,
  kDirtMiddleLeft = 11,
  kDirtMiddleMiddle = 12,
  kDirtMiddleRight = 13,
};

enum class CharacterAsset {
  kAll = 0,
  kVillager = 1,
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

static Rectf kDirtBottomLeftRect = Rectf(0.f, 153.f, kTW, kTH);
static Rectf kDirtBottomMiddleRect = Rectf(16.f, 153.f, kTW, kTH);
static Rectf kDirtBottomRightRect = Rectf(32.f, 153.f, kTW, kTH);

static Rectf kDirtMiddleLeftRect = Rectf(0.f, 137.f, kTW, kTH);
static Rectf kDirtMiddleMiddleRect = Rectf(16.f, 137.f, kTW, kTH);
static Rectf kDirtMiddleRightRect = Rectf(32.f, 137.f, kTW, kTH);

constexpr r32 kCW = 16;
constexpr r32 kCH = 16;

static Rectf kVillagerRect = Rectf(0.f, 16.f, kCW, kCH);

struct AssetStorage {
  u32 terrain_texture_id;
  u32 character_texture_id;
};

static AssetStorage kAssets;

void
AssetLoadAll()
{
  rgg::TextureInfo tinfo;
  tinfo.min_filter = GL_NEAREST_MIPMAP_NEAREST;
  tinfo.mag_filter = GL_NEAREST;
  // terrain is 256 by 256
  // rows: 16 cols: 16
  kAssets.terrain_texture_id = rgg::LoadTexture("asset/terrain.tga", tinfo);
  assert(kAssets.terrain_texture_id != 0);

  kAssets.character_texture_id = rgg::LoadTexture("asset/characters.tga", tinfo);
  assert(kAssets.character_texture_id != 0);
}

void
AssetTerrainRender(rgg::Texture* texture, v2f pos = v2f(0.f, 0.f), TerrainAsset asset = TerrainAsset::kAll)
{
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
    case TerrainAsset::kDirtBottomLeft: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kDirtBottomLeftRect, dest);
    } break;
    case TerrainAsset::kDirtBottomMiddle: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kDirtBottomMiddleRect, dest);
    } break;
    case TerrainAsset::kDirtBottomRight: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kDirtBottomRightRect, dest);
    } break;
    case TerrainAsset::kDirtMiddleLeft: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kDirtMiddleLeftRect, dest);
    } break;
    case TerrainAsset::kDirtMiddleMiddle: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kDirtMiddleMiddleRect, dest);
    } break;
    case TerrainAsset::kDirtMiddleRight: {
      Rectf dest = Rectf(pos.x, pos.y, kTW, kTH);
      rgg::RenderTexture(*texture, kDirtMiddleRightRect, dest);
    } break;


    default:
      assert(!"Implement rendering for this asset.");
      break;
  }
}

void
AssetCharacterRender(rgg::Texture* texture, v2f pos = v2f(0.f, 0.f), CharacterAsset asset = CharacterAsset::kAll)
{
  switch (asset) {
    case CharacterAsset::kAll: {
      Rectf rect(pos.x, pos.y, texture->width, texture->height);
      rgg::RenderTexture(*texture, rect, rect);
    } break;
    case CharacterAsset::kVillager: {
      Rectf dest = Rectf(pos.x, pos.y, kCW, kCH);
      rgg::RenderTexture(*texture, kVillagerRect, dest);
    } break;;
    default:
      assert(!"Implement rendering for this asset.");
      break;

  }
}

// }
