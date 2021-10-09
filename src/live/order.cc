// namespace live {

DEFINE_CALLBACK(HarvestCompleted, u32);

struct Order {
  enum Type {
    kMove = 0,
    kHarvest = 1,
  };
  Order(u32 id, Type type, v2f pos, int max_acquire_count) :
      id(id), type(type), pos(pos), max_acquire_count(max_acquire_count) {}
  Order(u32 id, Type type, u32 entity_id, int max_acquire_count) :
      id(id), type(type), entity_id(entity_id), max_acquire_count(max_acquire_count) {}
  u32 id = 0;
  union {
    u32 entity_id;
    v2f pos;
  };
  Type type;
  // Number of units that have acquired this order.
  u32 acquire_count = 0;
  u32 max_acquire_count = 0;
};

static u32 kInvalidOrderId = 0;
static u32 kOrderId = 1;
static std::unordered_map<s32, Order> kOrders;

Order*
_GetOrder(CharacterComponent* character)
{
  auto character_order = kOrders.find(character->order_id);

  if (character_order == kOrders.end()) {
    return nullptr;
  }

  return &character_order->second;
}

b8
_ExecuteMove(CharacterComponent* character, PhysicsComponent* phys, v2f target)
{
  v2f diff = target - phys->pos;
  r32 diff_lsq = math::LengthSquared(diff);
  if (diff_lsq > 1.f) {
    v2f dir = math::Normalize(diff);
    phys->pos += (dir * kCharacterDefaultSpeed);
    return false;
  }
  return true;
}

b8
_ExecuteMove(CharacterComponent* character, PhysicsComponent* phys, Order* order)
{
  assert(order->type == Order::kMove);
  return _ExecuteMove(character, phys, order->pos);
}

b8
_ExecuteHarvest(CharacterComponent* character, PhysicsComponent* physics, Order* order)
{
  assert(order->type == Order::kHarvest);

  Entity* harvest_entity = FindEntity(order->entity_id);
  assert(harvest_entity != nullptr);

  PhysicsComponent* tree_physics = GetPhysicsComponent(harvest_entity);
  assert(tree_physics != nullptr);

  if (_ExecuteMove(character, physics, tree_physics->pos)) {
    // TODO: Maybe add some mechanics for taking an amount of time to harvest the tree.
    DispatchHarvestCompleted(order->entity_id);
    return true;
  }

  return false;
}

void
OrderCreateMove(v2f pos)
{
  u32 oid = kOrderId++;
  kOrders.insert({oid, Order(oid, Order::kMove, pos, 1)});
}

void
OrderCreateHarvest(Entity* entity)
{
  for (const auto& [id, order] : kOrders)
  {
    if (order.type != Order::kHarvest) continue;
    // Order already created to harvest this thing. Don't try again.
    // TODO: Maybe store that information on the thing to be harvested itself for quicker lookup?
    if (order.entity_id == entity->id) return;
  }
  u32 oid = kOrderId++;
  kOrders.insert({oid, Order(oid, Order::kHarvest, entity->id, 1)});
}

void
OrderAcquire(CharacterComponent* character)
{
  Order* order = _GetOrder(character);

  if (order != nullptr) {
    return;
  }

  // Perhaps run some conditional logic on what types of orders a character is willing to acquire. 
  for (auto& p : kOrders) {
    if (p.second.acquire_count >= p.second.max_acquire_count) continue;
    character->order_id = p.first;
    ++(p.second.acquire_count);
    break;
  }
}

void
OrderExecute(EntityItr<2>* itr)
{
  CharacterComponent* character = itr->c.character;
  PhysicsComponent* physics = itr->c.physics;

  Order* order = _GetOrder(character);

  if (order == nullptr) {
    return;
  }

  b8 order_completed = false;
  switch (order->type) {
    case Order::kMove: {
      order_completed = _ExecuteMove(character, physics, order);
    } break;
    case Order::kHarvest: {
      order_completed = _ExecuteHarvest(character, physics, order);
    } break;
    default: break;
  }

  if (order_completed) {
    kOrders.erase(order->id);
    character->order_id = kInvalidOrderId;
  }
}


// }
