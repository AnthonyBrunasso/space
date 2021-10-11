// namespace live {

bool
GridPosIsValid(v2f pos)
{
  return pos.x >= 0.f && pos.y >= 0.f; // Also check if it's within max grid bounds?
}

bool
GridXYFromPos(v2f pos, v2i* xy)
{
  if (!GridPosIsValid(pos)) return false;
  xy->x = pos.x / kCellWidth;
  xy->y = pos.y / kCellHeight;
  return true;
}

v2f
GridPosFromXY(v2i xy)
{
  return v2f(xy.x * kCellWidth, xy.y * kCellHeight);
}

bool
GridClampPos(v2f pos, v2f* clamped_pos)
{
  v2i xy;
  if (!GridXYFromPos(pos, &xy)) return false;
  *clamped_pos = GridPosFromXY(xy);
  return true;
}


// }
