#define ENTITY_COUNT 512
#include "ecs/ecs.cc"

enum TypeId : u64 {
  kFoo = 0,
  kBar = 1,
};

struct Foo {
  u32 entity_id = 0;  
  u64 health = 0;
};

struct Bar {
  u32 entity_id = 0;
  r32 stuff = 0.f;
};

// Users must define their ecs nonsense in the ecs namespace.

namespace ecs {

ComponentStorage*
GetComponents(u64 tid)
{
  switch (tid) {
    case kFoo: {
      static ecs::ComponentStorage f(2, sizeof(Foo));
      return &f;
    } break;
    case kBar: {
      static ecs::ComponentStorage f(2, sizeof(Bar));
      return &f;
    } break;
    default: {
      assert(!"Unknown component type");
    } break;
  }
  return nullptr;
}

DECLARE_COMPONENT(Foo, kFoo);
DECLARE_COMPONENT(Bar, kBar);

// Must match order for TypeId.
struct Components {
  Foo* foo = nullptr;
  Bar* bar = nullptr;
};

}

int
main(int argc, char** argv)
{
  memory::Initialize(2048);

  ecs::Entity* e1 = ecs::UseEntity();
  ecs::Entity* e2 = ecs::UseEntity();

  {
    Foo* foo = ecs::AssignFoo(e1);
    foo->health = 32;
    Bar* bar = ecs::AssignBar(e1);
    bar->stuff = 2.3f;
  }

  {
    Foo* foo = ecs::AssignFoo(e2);
    foo->health = 16;
  }

  {
    ecs::EntityItr<2> itr(kFoo, kBar);
    while (itr.Next()) {
      printf("Entity found %u %u %.2f\n",
             itr.eid, itr.c.foo->health, itr.c.bar->stuff);
    }
  }

  {
    ecs::EntityItr<1> itr(kFoo);
    while (itr.Next()) {
      printf("Entity found %u %u\n", itr.eid, itr.c.foo->health);
    }
  }


  return 0;
}
