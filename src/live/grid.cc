// namespace live {

struct Cell {
  // A cell can contain, and also share, entity ids across multiple grid cells.
  // This happens when a structure takes up multiple grid cells.
  std::vector<u32> entity_ids;
  v2i pos;

  Rectf
  rect() const
  {
    return Rectf(GridPosFromXY(pos), v2f(kCellWidth, kCellHeight));
  }
};

struct Grid {
  Grid(v2i xy) :
    width(xy.x), height(xy.y), storage(xy.x * xy.y)
  {
    for (s32 x = 0; x < xy.x; ++x) {
      for (s32 y = 0; y < xy.y; ++y) {
        Cell* cell = Get(v2i(x, y));
        assert(cell != nullptr);
        cell->pos = v2i(x, y);
      }
    }
  }

  u32
  Index(v2i xy) const
  {
    u32 index = xy.y * width + xy.x;
    assert(index < storage.size());
    return index;
  }

  Cell*
  Get(v2i xy)
  {
    if (xy.x < 0 || xy.x > width) return nullptr;
    if (xy.y < 0 || xy.y > height) return nullptr;
    return &storage[Index(xy)];
  }

  std::vector<v2i>
  NeighborsPos(v2i xy)
  {
    std::vector<v2i> cells;
    cells.reserve(8);
    Cell* top_left = Get(xy + v2i(-1, 1));
    Cell* top = Get(xy + v2i(0, 1));
    Cell* top_right = Get(xy + v2i(1, 1));
    Cell* right = Get(xy + v2i(1, 0));
    Cell* bottom_right = Get(xy + v2i(1, -1));
    Cell* bottom = Get(xy + v2i(0, -1));
    Cell* bottom_left = Get(xy + v2i(-1, -1));
    Cell* left = Get(xy + v2i(-1, 0));
    if (top_left) cells.push_back(top_left->pos); 
    if (top) cells.push_back(top->pos); 
    if (top_right) cells.push_back(top_right->pos); 
    if (right) cells.push_back(right->pos); 
    if (bottom_right) cells.push_back(bottom_right->pos); 
    if (bottom) cells.push_back(bottom->pos); 
    if (bottom_left) cells.push_back(bottom_left->pos); 
    if (left) cells.push_back(left->pos); 
    return cells;
  }


  std::vector<Cell*>
  Neighbors(v2i xy)
  {
    std::vector<Cell*> cells;
    cells.reserve(8);
    Cell* top_left = Get(xy + v2i(-1, 1));
    Cell* top = Get(xy + v2i(0, 1));
    Cell* top_right = Get(xy + v2i(1, 1));
    Cell* right = Get(xy + v2i(1, 0));
    Cell* bottom_right = Get(xy + v2i(1, -1));
    Cell* bottom = Get(xy + v2i(0, -1));
    Cell* bottom_left = Get(xy + v2i(-1, -1));
    Cell* left = Get(xy + v2i(-1, 0));
    if (top_left) cells.push_back(top_left); 
    if (top) cells.push_back(top); 
    if (top_right) cells.push_back(top_right); 
    if (right) cells.push_back(right); 
    if (bottom_right) cells.push_back(bottom_right); 
    if (bottom) cells.push_back(bottom); 
    if (bottom_left) cells.push_back(bottom_left); 
    if (left) cells.push_back(left); 
    return cells;
  }

  std::vector<Cell*>
  Neighbors(const Cell* cell)
  {
    return Neighbors(cell->pos);
  }

  // Number of cells x
  u32 width;
  // Number of cells y
  u32 height;

  std::vector<Cell> storage;
};


std::vector<Grid> kGrids;

u32
GridCreate(v2i xy)
{
  kGrids.push_back(Grid(xy));
  // Grid ids are defined as kGrids[id - 1] to maintain 0 being unassigned.
  return kGrids.size();
}

Grid*
GridGet(u32 grid_id)
{
  assert(grid_id != 0); // Never ask for the invalid grid.
  u32 idx = grid_id - 1;
  assert(idx < kGrids.size());
  return &kGrids[idx];
}

std::vector<v2i>
GridGetIntersectingCellPos(PhysicsComponent* phys)
{
  Grid* grid = GridGet(phys->grid_id);
  assert(grid != nullptr);
  std::vector<v2i> nodes;

  v2i xy;
  if (GridXYFromPos(phys->pos, &xy)) {
    BfsSearchItr itr(xy,
        [grid, phys](v2i node) -> bool {
          Rectf grid_rect(GridPosFromXY(node), CellDims());
          Rectf character_rect = phys->rect();
          return math::IntersectRect(character_rect, grid_rect) ||
                 math::IsContainedInRect(character_rect, grid_rect) ||
                 math::IsContainedInRect(grid_rect, character_rect);
        },
        [grid](v2i node) -> std::vector<v2i> {
          return grid->NeighborsPos(node);
        }
    );
    while (itr.Next());
    nodes = std::move(itr.nodes);
  }

  return nodes;
}

void
GridSetEntity(PhysicsComponent* phys)
{
  v2i xy;
  if (!GridXYFromPos(phys->pos, &xy)) {
    assert(!"GridSetEntity requires a valid grid location.");
    return; 
  }

  Grid* grid = GridGet(phys->grid_id);
  std::vector<v2i> cells = GridGetIntersectingCellPos(phys);
  for (v2i cell : cells) {
    Cell* gcell = grid->Get(cell);
    assert(gcell != nullptr);
    gcell->entity_ids.push_back(phys->entity_id);
    //printf("grid->Set(%u, %u)->AddEntity(%u)\n", cell.x, cell.y, phys->entity_id);
  }
}

void
GridUnsetEntity(PhysicsComponent* phys)
{
  v2i xy;
  if (!GridXYFromPos(phys->pos, &xy)) {
    assert(!"GridSetEntity requires a valid grid location.");
    return; 
  }

  Grid* grid = GridGet(phys->grid_id);
  std::vector<v2i> cells = GridGetIntersectingCellPos(phys);
  for (v2i cell : cells) {
    Cell* gcell = grid->Get(cell);
    assert(gcell != nullptr);
    gcell->entity_ids.erase(std::remove(
          gcell->entity_ids.begin(), gcell->entity_ids.end(), phys->entity_id),
            gcell->entity_ids.end());
    //printf("grid->Set(%u, %u)->AddEntity(%u)\n", cell.x, cell.y, phys->entity_id);
  }
}

struct GridSync {
  GridSync(PhysicsComponent* phys) : phys(phys)
  {
    assert(phys != nullptr);
    nodes = GridGetIntersectingCellPos(phys);
  }

  ~GridSync()
  {
    assert(phys != nullptr);
    Grid* grid = GridGet(phys->grid_id);
    assert(grid != nullptr);
    std::vector<v2i> new_nodes = GridGetIntersectingCellPos(phys);
    if (nodes != new_nodes) {
      // Unset all from original nodes.
      for (v2i node : nodes) {
        Cell* gcell = grid->Get(node);
        assert(gcell != nullptr);
        gcell->entity_ids.erase(std::remove(
          gcell->entity_ids.begin(), gcell->entity_ids.end(), phys->entity_id),
            gcell->entity_ids.end());
        //printf("grid(%u, %u)->RemoveEntity(%u)\n", node.x, node.y, phys->entity_id);
      }
      for (v2i node : new_nodes) {
        Cell* gcell = grid->Get(node);
        assert(gcell != nullptr);
        gcell->entity_ids.push_back(phys->entity_id);
        //printf("grid(%u, %u)->AddEntity(%u)\n", node.x, node.y, phys->entity_id);
      }
    }
  }

  std::vector<v2i> nodes;
  PhysicsComponent* phys;
};

// }
