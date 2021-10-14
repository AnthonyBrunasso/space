// namespace live {

void
SimCreateHarvest(ResourceType resource_type, v2f pos, u32 grid_id, r32 seconds_to_harvest)
{
  assert(GridPosIsValid(pos));
  Entity* entity = UseEntity();

  PhysicsComponent* phys = AssignPhysicsComponent(entity);
  phys->pos = pos;
  phys->grid_id = grid_id;

  switch (resource_type) {
    case kLumber:
      phys->bounds = v2f(live::kLumberWidth, live::kLumberHeight);
      break;
    case kStone:
      phys->bounds = v2f(live::kStoneWidth, live::kStoneHeight);
      break;
    case kResourceTypeCount:
    default:
      break;
  }

  HarvestComponent* harvest = AssignHarvestComponent(entity);
  harvest->tth = SecondsToTicks(seconds_to_harvest);

  ResourceComponent* resource = AssignResourceComponent(entity);
  resource->resource_type = resource_type;

  GridSetEntity(phys);
}

void
SimCreateCharacter(v2f pos, u32 grid_id)
{
  assert(GridPosIsValid(pos));
  Entity* character = UseEntity();

  PhysicsComponent* phys = AssignPhysicsComponent(character);
  phys->pos = pos;
  phys->bounds = v2f(live::kCharacterWidth, live::kCharacterHeight);
  phys->grid_id = grid_id;

  AssignCharacterComponent(character);

  GridSetEntity(phys);
}

void
SimCreateWall(v2f pos, u32 grid_id)
{
  assert(GridPosIsValid(pos));
  Entity* wall = UseEntity();

  PhysicsComponent* phys = AssignPhysicsComponent(wall);
  phys->pos = pos;
  phys->bounds = v2f(live::kWallWidth, live::kWallHeight);
  phys->grid_id = grid_id;

  StructureComponent* structure = AssignStructureComponent(wall);
  structure->structure_type = kWall; 

  GridSetEntity(phys);
}

void
SimCreateHarvestOrder(Entity* harvest_entity)
{
  ECS_ITR1(itr, kOrderComponent);
  while (itr.Next()) {
    if (itr.e->id == harvest_entity->id)
      return;
  }

  OrderComponent* order = AssignOrderComponent(harvest_entity);
  order->order_type = kHarvest;
  order->acquire_count  = 0;
  order->max_acquire_count = 1;
}

void
SimCreateBuildOrder(StructureType structure_type, v2f pos, u32 grid_id, r32 seconds_to_build)
{
  v2f grid_pos;
  if (!GridClampPos(pos, &grid_pos)) {
    //assert(!"Build order created out of bounds of grid");
    return;
  }
  assert(GridPosIsValid(pos));
  Entity* entity = UseEntity();

  PhysicsComponent* phys = AssignPhysicsComponent(entity);
  phys->pos = grid_pos;
  phys->grid_id = grid_id;

  switch (structure_type) {
    case kWall:
      phys->bounds = v2f(live::kWallWidth, live::kWallHeight);
      break;
    case kStructureTypeCount:
    default:
      break;
  }

  BuildComponent* build = AssignBuildComponent(entity);
  build->structure_type = structure_type;
  build->ttb = SecondsToTicks(seconds_to_build);
  build->required_resource_type = kLumber;
  build->resource_count = 1;

  OrderComponent* order = AssignOrderComponent(entity);
  order->order_type = kBuild;
  order->acquire_count  = 0;
  order->max_acquire_count = 1;

  GridSetEntity(phys);
}

void
SimCreateZone(const Rectf& selection, u32 grid_id)
{
  Rectf aabb = math::OrientToAabb(selection);
  v2f min_world = aabb.Min();
  v2f max_world = aabb.Max();
  v2i min_grid, max_grid;
  if (!GridXYFromPos(min_world, &min_grid)) {
    printf("ZoneBoxSelect: Invalid grid min...\n");
    return;
  }

  if (!GridXYFromPos(max_world, &max_grid)) {
    printf("ZoneBoxSelect: Invalid grid max...\n");
    return;
  }

  Entity* entity = UseEntity();

  PhysicsComponent* phys = AssignPhysicsComponent(entity);
  phys->pos = GridPosFromXY(min_grid);
  // TODO: This isn't quite right.
  phys->bounds = v2f((max_grid.x - min_grid.x + 1) * kCellWidth,
                     (max_grid.y - min_grid.y + 1) * kCellHeight);
  phys->grid_id = grid_id;

  ZoneComponent* zone = AssignZoneComponent(entity);
  zone->zone_start = min_grid;
  zone->zone_end = max_grid;
  // TODO: Probably drive this from UI.
  zone->resource_mask = FLAG(kLumber) | FLAG(kStone);

  GridSetEntity(phys);
}

// }
