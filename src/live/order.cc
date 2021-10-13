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

void
OrderAcquire(CharacterComponent* character)
{
  OrderComponent* order = _GetOrder(character);

  if (order != nullptr) {
    return;
  }

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
    }
    character->order_id = order->entity_id;
    ++(order->acquire_count);
    break;
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
    default: break;
  }

  if (order_completed) {
    //RemoveOrderComponent(itr->e);
    character->order_id = 0;
  }
}


// }
