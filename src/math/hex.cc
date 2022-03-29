#pragma once

#include <vector>

namespace math {

static const r32 kSqrt3 = sqrt(3.f);
static const r32 kSqrt3Div2 = kSqrt3 / 2.f;
static const r32 kSqrt3Div3 = sqrt(3.f) / 3.f;

static v2i kHexAxialNeighbors[6] = {
  {1, 0}, {0, 1}, {-1, 1}, {-1, 0}, {0, -1}, {1, -1}
};

static v3i kHexCubeNeighbors[6] = {
  {1,0,-1}, {0,1,-1}, {-1,1,0}, {-1,0,1}, {0,-1,1}, {1,-1,0},
};

v2f HexCorner(const v2f& center, r32 size, s32 corner) {
  assert(corner <= 6);
  return center + Rotate(v2f(size, 0.f), corner * 60.f - 30.f);
}

v2i HexCubeToAxial(const v3i& cube) {
  return v2i(cube.x, cube.z);
}

v3i HexAxialToCube(const v2i& axial) {
  return v3i(axial.x, -axial.x - axial.y, axial.y);
}

v3f HexAxialToCubef(const v2f& axial) {
  return v3f(axial.x, -axial.x - axial.y, axial.y);
}

v2f HexAxialToWorld(const v2i& axial, r32 size) {
  return v2f(size * (kSqrt3 * axial.x + kSqrt3Div2 * axial.y),
             size * (3.f / 2.f * axial.y));
}

v2i HexAxialNeighbor(const v2i& axial, u32 i) {
  assert(i < 6);
  return axial + kHexAxialNeighbors[i];
}

v3i HexCubeNeighbor(const v3i& cube, u32 i) {
  assert(i >= 0 && i < 6);
  return cube + kHexCubeNeighbors[i];
}

r32 HexCubeDistance(const v3i& a, const v3i& b) {
  return (abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z)) / 2.f;
}

r32 HexAxialDistance(const v2i& a, const v2i& b) {
  return HexCubeDistance(HexAxialToCube(a), HexAxialToCube(b));
}

std::vector<v2i> HexAxialRange(s32 n) {
  std::vector<v2i> range;
  for (int x = -n; x <= n; ++x) {
    for (int y = -n; y <= n; ++y) {
      for (int z = -n; z <= n; ++z) {
        if (x + y + z == 0) {
          range.push_back(HexCubeToAxial(v3i(x, y, z)));
        }
      }
    }
  }
  return range;
}

std::vector<v3i> HexCubeRange(s32 n) {
  std::vector<v3i> range;
  for (int x = -n; x <= n; ++x) {
    for (int y = -n; y <= n; ++y) {
      for (int z = -n; z <= n; ++z) {
        if (x + y + z == 0) {
          range.emplace_back(x, y, z);
        }
      }
    }
  }
  return range;
}

v3i HexCubeRound(const v3f& cube) {
  r32 rx = roundf(cube.x);
  r32 ry = roundf(cube.y);
  r32 rz = roundf(cube.z);

  r32 xdiff = fabs(rx - cube.x);
  r32 ydiff = fabs(ry - cube.y);
  r32 zdiff = fabs(rz - cube.z);

  if (xdiff > ydiff && xdiff > zdiff) {
    rx = -ry - rz;
  } else if (ydiff > zdiff) {
    ry = -rx - rz;
  } else {
    rz = -rx - ry;
  }
  return v3i((s32)rx, (s32)ry, (s32)rz);
}

v2i HexAxialRound(const v2f& axial) {
  return HexCubeToAxial(HexCubeRound(HexAxialToCubef(axial)));
}

v2i HexWorldToAxial(const v2f& p, r32 size) {
  return HexAxialRound(
      v2f((kSqrt3Div3 * p.x - 1.f / 3.f * p.y) / size,
          (2.f / 3.f * p.y) / size));
}

}  // namespace math
