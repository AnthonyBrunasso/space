// namespace live {

void
SimCreateHarvest(ResourceType resource_type, v2f pos, r32 seconds_to_harvest)
{
  Entity* entity = UseEntity();
  PhysicsComponent* comp = AssignPhysicsComponent(entity);
  comp->pos = pos;
  switch (resource_type) {
    case kLumber:
      comp->bounds = v2f(live::kLumberWidth, live::kLumberHeight);
      break;
    case kStone:
      comp->bounds = v2f(live::kStoneWidth, live::kStoneHeight);
      break;
    case kResourceTypeCount:
    default:
      break;
  }
  HarvestComponent* harvest = AssignHarvestComponent(entity);
  harvest->resource_type = resource_type;
  harvest->tth = SecondsToTicks(seconds_to_harvest);
}

void
SimCreateCharacter(v2f pos)
{
  Entity* character = UseEntity();
  PhysicsComponent* comp = AssignPhysicsComponent(character);
  comp->pos = pos;
  comp->bounds = v2f(live::kCharacterWidth, live::kCharacterHeight);
  AssignCharacterComponent(character);
}

void
SimCreateWall(v2f pos)
{
  Entity* wall = UseEntity();
  PhysicsComponent* comp = AssignPhysicsComponent(wall);
  comp->pos = pos;
  comp->bounds = v2f(live::kWallWidth, live::kWallHeight);
  StructureComponent* structure = AssignStructureComponent(wall);
  structure->structure_type = kWall; 
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
SimCreateBuildOrder(StructureType structure_type, v2f pos, r32 seconds_to_build)
{
  Entity* entity = UseEntity();
  PhysicsComponent* comp = AssignPhysicsComponent(entity);
  comp->pos = pos - v2f(kWallWidth  / 2.f, kWallHeight / 2.f);
  switch (structure_type) {
    case kWall:
      comp->bounds = v2f(live::kWallWidth, live::kWallHeight);
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
}

// }
