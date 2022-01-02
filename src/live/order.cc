// namespace live {

DEFINE_CALLBACK(HarvestCompleted, u32);
DEFINE_CALLBACK(BuildCompleted, u32);

OrderComponent*
_GetOrder(CharacterComponent* character)
{
  Entity* order_entity = FindEntity(character->order_id);
  if (!order_entity) return nullptr;
  return GetOrderComponent(order_entity);
}

b8
OrderExecuteMove(CharacterComponent* character, PhysicsComponent* phys, v2f target)
{
  v2f diff = target - phys->pos;
  r32 diff_lsq = math::LengthSquared(diff);
  if (diff_lsq > 1.f) {
    GridSync grid_sync(phys);
    v2f dir = math::Normalize(diff);
    phys->pos += (dir * kCharacterDefaultSpeed);
    return false;
  }
  return true;
}

b8
OrderExecuteMove(CharacterComponent* character, PhysicsComponent* phys, OrderComponent* order)
{
  assert(order->order_type == kMove);
  Entity* order_entity = FindEntity(order->entity_id);
  assert(order_entity != nullptr);
  PhysicsComponent* order_physics = GetPhysicsComponent(order_entity);
  assert(order_physics != nullptr);
  return OrderExecuteMove(character, phys, order_physics->pos);
}

b8
OrderExecuteHarvest(CharacterComponent* character, PhysicsComponent* physics, OrderComponent* order)
{
  assert(order->order_type == kHarvest);

  Entity* harvest_entity = FindEntity(order->entity_id);
  assert(harvest_entity != nullptr);

  PhysicsComponent* tree_physics = GetPhysicsComponent(harvest_entity);
  assert(tree_physics != nullptr);

  HarvestComponent* harvest = GetHarvestComponent(harvest_entity);
  assert(harvest != nullptr);

  if (OrderExecuteMove(character, physics, tree_physics->pos)) {
    if (harvest->tth > 0 ) {
      --harvest->tth;
    } else {
      DispatchHarvestCompleted(order->entity_id);
      return true;
    }
  }

  return false;
}

b8
OrderExecuteBuild(CharacterComponent* character, PhysicsComponent* physics, OrderComponent* order)
{
  assert(order->order_type == kBuild);

  Entity* build_entity = FindEntity(order->entity_id);
  assert(build_entity != nullptr);

  PhysicsComponent* build_physics = GetPhysicsComponent(build_entity);
  assert(build_physics != nullptr);

  BuildComponent* build = GetBuildComponent(build_entity);
  assert(build != nullptr);

  if (OrderExecuteMove(character, physics, build_physics->pos)) {
    if (build->ttb > 0 ) {
      --build->ttb;
    } else {
      DispatchBuildCompleted(build->entity_id);
      for (u32 req_entity_id : build->requesite_entity_ids) {
        AssignDeathComponent(req_entity_id);
      }
      return true;
    }
  }

  return false;
}

void
OrderMorphToCarryToZone(OrderComponent* order, PhysicsComponent* physics)
{
  // Find the zone to carry this thing to.
  ECS_ITR2(itr, kZoneComponent, kPhysicsComponent);
  ZoneComponent* zone = nullptr;
  PhysicsComponent* zone_physics = nullptr;
  while (itr.Next()) {
    // TODO: Check that zone can hold this resource??
    if (ZoneHasCapacity(itr.c.zone)) {
      zone = itr.c.zone;
      zone_physics = itr.c.physics;
      break;
    }
  }
  assert(zone != nullptr);
  assert(zone_physics != nullptr);
  Grid* grid = GridGet(zone_physics->grid_id);
  assert(grid != nullptr);
  ZoneCell* use_cell = nullptr;
  for (ZoneCell& zcell : zone->zone_cells) {
    if (zcell.reserved) continue;
    // printf("Looking at cell %i %i\n", cell.x, cell.y);
    Cell* gcell = grid->Get(zcell.grid_pos);
    assert(gcell != nullptr);
    // If the cell does not have a resource or building entity.
    bool is_cell_valid = true;
    for (u32 entity_id : gcell->entity_ids) {
      Entity* entity = FindEntity(entity_id);
      if (entity->Has(kResourceComponent) || entity->Has(kBuildComponent)) {
        is_cell_valid = false;
        break;
      }
    }
    if (is_cell_valid) {
      use_cell = &zcell;
      break;
    }
  }
  // TODO: Temp... What should we do if we get here and don't have a valid cell to take the item to.
  assert(use_cell != nullptr);
  if (use_cell) {
    order->carry_to_data.grid_pos = use_cell->grid_pos;
    use_cell->reserved = true;
  }
}

b8
OrderExecutePickup(CharacterComponent* character, PhysicsComponent* physics, OrderComponent* order)
{
  assert(order->order_type == kPickup);

  Entity* pickup_entity = FindEntity(order->entity_id);
  assert(pickup_entity != nullptr);

  PhysicsComponent* pickup_physics = GetPhysicsComponent(pickup_entity);
  assert(pickup_physics != nullptr);

  if (OrderExecuteMove(character, physics, pickup_physics->pos)) {
    CarryComponent* carry = AssignCarryComponent(pickup_entity);
    carry->carrier_entity_id = character->entity_id;
    RemovePickupComponent(pickup_entity);
    character->carrying_id = pickup_entity->id;
    order->order_type = kCarryTo;
    if (order->pickup_data.destination == PickupData::kFindZone) {
      OrderMorphToCarryToZone(order, physics);
    } else if (order->pickup_data.destination == PickupData::kBuild) {
      Entity* build_entity = FindEntity(order->pickup_data.build_entity_id);
      assert(build_entity != nullptr);
      PhysicsComponent* build_physics = GetPhysicsComponent(build_entity);
      assert(build_physics != nullptr);
      std::vector<v2i> cells = GridGetIntersectingCellPos(build_physics);
      assert(cells.size() > 0);
      order->carry_to_data.grid_pos = cells[0];
    }
  }

  // Pickup never returns true because pickups are only issued to be carried somewhere else. Instead
  // mutate the order in place and the character will begin carrying to the target.
  return false;
}

b8
OrderExecuteCarryTo(CharacterComponent* character, PhysicsComponent* physics, OrderComponent* order)
{
  v2f pos = GridPosFromXY(order->carry_to_data.grid_pos);
  if (OrderExecuteMove(character, physics, pos)) {
    Entity* carried_entity = FindEntity(character->carrying_id);
    assert(carried_entity != nullptr);
    PhysicsComponent* carried_physics = GetPhysicsComponent(carried_entity);
    assert(carried_physics != nullptr);
    GridSync gsync(carried_physics);
    carried_physics->pos = pos;
    RemoveCarryComponent(carried_entity);
    character->carrying_id = 0;
    // Assume a resource was carried to a stockpile???
    if (carried_entity->Has(kResourceComponent)) {
      ResourceComponent* resource = GetResourceComponent(carried_entity);
      ++kSim.resources[resource->resource_type];
    }
    return true;
  }
  return false;
}

bool
CanBuildProceed(Entity* ent, BuildComponent* build_comp, std::vector<u32>* entity_ids)
{
  ResourceType type = build_comp->required_resource_type;
  u32 count = build_comp->resource_count;
  // Building things requires resources?
  assert(count > 0);
  PhysicsComponent* physics = GetPhysicsComponent(ent);
  assert(physics != nullptr);
  std::vector<v2i> grid_cells = GridGetIntersectingCellPos(physics);
  Grid* grid = GridGet(physics->grid_id);
  assert(grid != nullptr);
  for (v2i gcell : grid_cells) {
    Cell* cell = grid->Get(gcell);
    assert(cell != nullptr);
    for (u32 entity_id : cell->entity_ids) {
      Entity* cell_entity = FindEntity(entity_id);
      assert(cell_entity != nullptr);
      if (!cell_entity->Has(kResourceComponent)) continue;
      ResourceComponent* resource = GetResourceComponent(cell_entity);
      assert(resource != nullptr);
      if (resource->resource_type == type) {
        entity_ids->push_back(entity_id);
        --count;
      }
      if (!count) break;
    }
    if (!count) break;
  }
  // Build is satisfied if the count of resources available exist at the location.
  return count == 0;
}

void
OrderCreatePickupsFor(BuildComponent* build_comp)
{
  ECS_ITR2(itr, kZoneComponent, kPhysicsComponent);
  ResourceComponent* resource_component = nullptr;
  while (itr.Next()) {
    if (ZoneHasResourceForPickup(itr.c.zone, &resource_component, build_comp->required_resource_type)) {
      break;
    }
  }
  if (resource_component) {
    Entity* resource_entity = FindEntity(resource_component->entity_id);
    assert(resource_entity);
    AssignPickupComponent(resource_entity);
    // Consider abstracting order creation to a sim_create function perhaps.
    OrderComponent* order = AssignOrderComponent(resource_entity);
    order->order_type = kPickup;
    order->acquire_count = 0;
    order->max_acquire_count = 1;
    order->pickup_data.build_entity_id = build_comp->entity_id;
    order->pickup_data.destination = PickupData::kBuild;
    build_comp->pickup_orders_issued += 1;
  }
}

void
OrderAcquire(CharacterComponent* character)
{
  OrderComponent* order = _GetOrder(character);

  if (order != nullptr) {
    return;
  }

  {
    // Perhaps run some conditional logic on what types of orders a character is willing to acquire. 
    ECS_ITR1(itr, kOrderComponent);
    while (itr.Next()) {
      OrderComponent* order = itr.c.order;
      if (order->acquire_count >= order->max_acquire_count) {
        continue;
      }
      if (order->order_type == kBuild) {
        BuildComponent* build_comp = GetBuildComponent(itr.e);
        assert(build_comp != nullptr);
        // Don't have enough resources to build.
        if (kSim.resources[build_comp->required_resource_type] < build_comp->resource_count) {
          continue;
        }

        b8 can_pickups_be_issued = build_comp->pickup_orders_issued < build_comp->resource_count;
        if (can_pickups_be_issued) {
          OrderCreatePickupsFor(build_comp);
          continue;
        }

        //if (!character->HasJob(Job::kBuild))
        //  continue;

        //printf("Can anuyone build???\n");

        std::vector<u32> entity_ids;
        b8 can_build_proceed = CanBuildProceed(itr.e, build_comp, &entity_ids);
        // Check if the build order can proceed.
        if (!can_build_proceed) {
          continue;
        }
        build_comp->requesite_entity_ids = entity_ids;
      } else if (order->order_type == kPickup) {
        if (!character->HasJob(Job::kHaul))
          continue;

        // Look for a zone that this thing can be moved to.
        ResourceComponent* resource = GetResourceComponent(itr.e);
        if (resource) {
          ECS_ITR1(itr, kZoneComponent);
          bool valid_zone = false;
          while (itr.Next()) {
            valid_zone = true; 
          }
          if (!valid_zone) continue;
        }
      } else if (order->order_type == kHarvest) {
        if (!character->HasJob(Job::kHarvest))
          continue;
      }
      character->order_id = order->entity_id;
      ++(order->acquire_count);
      break;
    }
  }
}

void
OrderExecute(EntityItr<2>* itr)
{
  CharacterComponent* character = itr->c.character;
  PhysicsComponent* physics = itr->c.physics;

  OrderComponent* order = _GetOrder(character);

  if (order == nullptr) {
    return;
  }

  OrderType original_order_type = order->order_type;

  b8 order_completed = false;
  switch (order->order_type) {
    case kMove: {
      order_completed = OrderExecuteMove(character, physics, order);
    } break;
    case kHarvest: {
      order_completed = OrderExecuteHarvest(character, physics, order);
    } break;
    case kBuild: {
      order_completed = OrderExecuteBuild(character, physics, order);
    } break;
    case kPickup: {
      order_completed = OrderExecutePickup(character, physics, order);
    } break;
    case kCarryTo: {
      order_completed = OrderExecuteCarryTo(character, physics, order);
    } break;
    default: break;
  }

  // NOTE: The pointer to order might be different after Execute*...

  if (order_completed) {
    // If the order hasn't changed get rid of it since it's done. If it has  changed the completion
    // of one order likely mutated the existing component. Idk if that's bad but that's how this works.
    if (original_order_type == order->order_type) {
      //printf("Removing order component %u\n", itr->e->id);
      Entity* order_entity = FindEntity(order->entity_id);
      RemoveOrderComponent(order_entity);
    }
    character->order_id = 0;
  }
}


// }
