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
_ExecuteMove(CharacterComponent* character, PhysicsComponent* phys, v2f target)
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
_ExecuteMove(CharacterComponent* character, PhysicsComponent* phys, OrderComponent* order)
{
  assert(order->order_type == kMove);
  Entity* order_entity = FindEntity(order->entity_id);
  assert(order_entity != nullptr);
  PhysicsComponent* order_physics = GetPhysicsComponent(order_entity);
  assert(order_physics != nullptr);
  return _ExecuteMove(character, phys, order_physics->pos);
}

b8
_ExecuteHarvest(CharacterComponent* character, PhysicsComponent* physics, OrderComponent* order)
{
  assert(order->order_type == kHarvest);

  Entity* harvest_entity = FindEntity(order->entity_id);
  assert(harvest_entity != nullptr);

  PhysicsComponent* tree_physics = GetPhysicsComponent(harvest_entity);
  assert(tree_physics != nullptr);

  HarvestComponent* harvest = GetHarvestComponent(harvest_entity);
  assert(harvest != nullptr);

  if (_ExecuteMove(character, physics, tree_physics->pos)) {
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
_ExecuteBuild(CharacterComponent* character, PhysicsComponent* physics, OrderComponent* order)
{
  assert(order->order_type == kBuild);

  Entity* build_entity = FindEntity(order->entity_id);
  assert(build_entity != nullptr);

  PhysicsComponent* build_physics = GetPhysicsComponent(build_entity);
  assert(build_physics != nullptr);

  BuildComponent* build = GetBuildComponent(build_entity);
  assert(build != nullptr);

  if (_ExecuteMove(character, physics, build_physics->pos)) {
    if (build->ttb > 0 ) {
      --build->ttb;
    } else {
      DispatchBuildCompleted(build->entity_id);
      return true;
    }
  }

  return false;
}

b8
_ExecutePickup(CharacterComponent* character, PhysicsComponent* physics, OrderComponent* order)
{
  assert(order->order_type == kPickup);

  Entity* pickup_entity = FindEntity(order->entity_id);
  assert(pickup_entity != nullptr);

  PhysicsComponent* pickup_physics = GetPhysicsComponent(pickup_entity);
  assert(pickup_physics != nullptr);

  if (_ExecuteMove(character, physics, pickup_physics->pos)) {
    //printf("Thingy picked up...\n");
    CarryComponent* carry = AssignCarryComponent(pickup_entity);
    carry->carrier_entity_id = character->entity_id;
    RemovePickupComponent(pickup_entity);
    character->carrying_id = pickup_entity->id;
    order->order_type = kCarryTo;
    // Find the zone to carry this thing to.
    ECS_ITR1(itr, kZoneComponent);
    bool zone_found = false;
    while (itr.Next()) {
      order->target_entity_id = itr.e->id;
      zone_found = true;
      break;
    }
    assert(zone_found != false);
  }

  // Pickup never returns true because pickups are only issued to be carried somewhere else. Instead
  // mutate the order in place and the character will begin carrying to the target.
  return false;
}

b8
_ExecuteCarryTo(CharacterComponent* character, PhysicsComponent* physics, OrderComponent* order)
{
  Entity* target_entity = FindEntity(order->target_entity_id);
  assert(target_entity != nullptr);
  PhysicsComponent* target_physics = GetPhysicsComponent(target_entity);
  assert(target_physics != nullptr);
  if (_ExecuteMove(character, physics, target_physics->pos)) {
    // Distribute resource to zone.
    ZoneComponent* zone_component = GetZoneComponent(target_entity);
    assert(zone_component != nullptr);
    std::vector<v2i> grid_cells = GridGetIntersectingCellPos(target_physics);
    assert(!grid_cells.empty());
    Grid* grid = GridGet(target_physics->grid_id);
    assert(grid != nullptr);
    bool can_use_cell = false;
    v2i use_cell;
    for (v2i cell : grid_cells) {
      Cell* gcell = grid->Get(cell);
      printf("Trying cell  %i %i\n", cell.x, cell.y);
      assert(gcell != nullptr);
      if (gcell->entity_ids.empty()) {
        can_use_cell = true;
        use_cell = gcell->pos;
        break;
      }
      // If there exist entities that don't block the resource from being put here.
      for (u32 entity_id : gcell->entity_ids) {
        Entity* entity = FindEntity(entity_id);
        assert(entity != nullptr);
        if (entity->Has(kResourceComponent) || entity->Has(kStructureComponent)) {
          printf("Has resource or structure %i %i\n", cell.x, cell.y);
          break;
        }
        use_cell = gcell->pos;
        can_use_cell = true;
        printf("Using cell %i %i\n", use_cell.x, use_cell.y);
        break;
      }
      if (can_use_cell) break;
    }
    if (can_use_cell) {
      Entity* carried_entity = FindEntity(character->carrying_id);
      assert(carried_entity != nullptr);
      PhysicsComponent* carried_physics = GetPhysicsComponent(carried_entity);
      assert(carried_physics != nullptr);
      GridSync gsync(carried_physics);
      printf("Place resource %i %i\n", use_cell.x, use_cell.y);
      carried_physics->pos = GridPosFromXY(use_cell);
      RemoveCarryComponent(carried_entity);
      character->carrying_id = 0;
      return true;
    }
  }
  return false;
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
      } else if (order->order_type == kPickup) {
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
  OrderType original_order_type = order->order_type;

  if (order == nullptr) {
    return;
  }

  b8 order_completed = false;
  switch (order->order_type) {
    case kMove: {
      order_completed = _ExecuteMove(character, physics, order);
    } break;
    case kHarvest: {
      order_completed = _ExecuteHarvest(character, physics, order);
    } break;
    case kBuild: {
      order_completed = _ExecuteBuild(character, physics, order);
    } break;
    case kPickup: {
      order_completed = _ExecutePickup(character, physics, order);
    } break;
    case kCarryTo: {
      order_completed = _ExecuteCarryTo(character, physics, order);
    } break;
    default: break;
  }

  // NOTE: The pointer to order might be different after Execute*...

  if (order_completed) {
    // If the order hasn't changed get rid of it since it's done. If it has  changed the completion
    // of one order likely mutated the existing component. Idk if that's bad but that's how this works.
    if (itr->e->Has(kOrderComponent) && (original_order_type != order->order_type)) {
      RemoveOrderComponent(itr->e);
    }
    character->order_id = 0;
  }
}


// }
