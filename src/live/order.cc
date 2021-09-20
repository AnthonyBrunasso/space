// namespace live {

struct Character;

struct Order {
  enum Type {
    MOVE = 0,
  };
  Order(u32 id, Type type, v2f pos, int max_acquire_count) :
      id(id), type(type), pos(pos), max_acquire_count(max_acquire_count) {}
  u32 id = 0;
  v2f pos;
  Type type;
  // Number of units that have acquired this order.
  u32 acquire_count = 0;
  u32 max_acquire_count = 0;
};

static u32 kInvalidOrderId = 0;
static u32 kOrderId = 1;
static std::unordered_map<s32, Order> kOrders;

Order*
_GetOrder(Character* character)
{
  auto character_order = kOrders.find(character->order_id);

  if (character_order == kOrders.end()) {
    return nullptr;
  }

  return &character_order->second;
}

b8
_ExecuteMove(Character* character, Order* order)
{
  v2f diff = order->pos - character->pos;
  r32 diff_lsq = math::LengthSquared(diff);
  if (diff_lsq > 1.f) {
    v2f dir = math::Normalize(diff);
    character->pos += (dir * 2.f);
    return false;
  }
  return true;
}

void
OrderCreateMove(v2f pos)
{
  u32 oid = kOrderId++;
  kOrders.insert({oid, Order(oid, Order::MOVE, pos, 1)});
}

void
OrderAcquire(Character* character)
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
OrderExecute(Character* character)
{
  Order* order = _GetOrder(character);

  if (order == nullptr) {
    return;
  }

  //puts("Execute order");
  b8 order_completed = false;
  switch (order->type) {
    case Order::MOVE: {
      order_completed = _ExecuteMove(character, order);
    } break;
    default: break;
  }

  if (order_completed) {
    kOrders.erase(order->id);
  }
}


// }
