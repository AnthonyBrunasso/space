namespace live {

constexpr s32 kScreenWidth = 1920;
constexpr s32 kScreenHeight = 1080;

constexpr r32 kLumberWidth = 10.f;
constexpr r32 kLumberHeight = 10.f;

// The width / height for cells in a grid. This will be used to map
// x / y positions to grid cells in grid.cc.
constexpr r32 kCellWidth = 15.f;
constexpr r32 kCellHeight = 15.f;

static v2f
CellDims()
{
  return v2f(kCellWidth, kCellHeight);
}

constexpr r32 kStoneWidth = kCellWidth;
constexpr r32 kStoneHeight = kCellHeight;

constexpr r32 kWallWidth = kCellWidth;
constexpr r32 kWallHeight = kCellHeight;

constexpr r32 kSecsToHarvestLumber = 3.5f;
constexpr r32 kSecsToHarvestStone = 10.f;

constexpr r32 kCharacterWidth = 10.f;
constexpr r32 kCharacterHeight = 10.f;
constexpr r32 kCharacterDefaultSpeed = 0.5f;

constexpr r32 kCameraSpeed = 10.f;

}
