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
  harvest->resource_type = resource_type;
  harvest->tth = SecondsToTicks(seconds_to_harvest);
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

// }
