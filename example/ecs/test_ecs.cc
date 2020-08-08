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
      static ecs::ComponentStorage f(8, sizeof(Foo));
      return &f;
    } break;
    case kBar: {
      static ecs::ComponentStorage f(8, sizeof(Bar));
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
  //while (!IsDebuggerPresent()) {}

  memory::Initialize(2048);

  ecs::Entity* e1 = ecs::UseEntity();
  ecs::Entity* e2 = ecs::UseEntity();
  ecs::Entity* e3 = ecs::UseEntity();
  ecs::Entity* e4 = ecs::UseEntity();
  ecs::Entity* e5 = ecs::UseEntity();

  ecs::AssignFoo(e1);
  ecs::AssignFoo(e2);
  ecs::AssignFoo(e3);
  ecs::AssignFoo(e4);

  ecs::AssignBar(e2);
  ecs::AssignBar(e3);
  ecs::AssignBar(e4);

  ecs::EntityItr<2> itr(kFoo, kBar);
  while (itr.Next()) {
    printf("Entity found %u\n", itr.e->id);
  }
#if 0
  {
    Foo* foo = ecs::AssignFoo(e4);
    foo->health = 4;
  }

  {
    Foo* foo = ecs::AssignFoo(e2);
    foo->health = 2;
  }

  {
    Foo* foo = ecs::AssignFoo(e3);
    foo->health = 3;
  }

  {
    Foo* foo = ecs::AssignFoo(e1);
    foo->health = 32;
    Bar* bar = ecs::AssignBar(e1);
    bar->stuff = 2.3f;
  }

  {
    Foo* foo = ecs::AssignFoo(e5);
    foo->health = 5;
  }

  {
    ecs::RemoveBar(e1);
    ecs::EntityItr<2> itr(kFoo, kBar);
    while (itr.Next()) {
      printf("Entity found %u health: %u stuff: %.2f\n",
             itr.eid, itr.c.foo->health, itr.c.bar->stuff);
    }
  }

  {
    ecs::EntityItr<1> itr(kFoo);
    while (itr.Next()) {
      printf("Entity found %u %u\n", itr.eid, itr.c.foo->health);
    }
  }

  printf("%u %u\n", e1->id, ecs::GetFoo(e1)->health);
  printf("%u %u\n", e2->id, ecs::GetFoo(e2)->health);
  printf("%u %u\n", e3->id, ecs::GetFoo(e3)->health);
  printf("%u %u\n", e4->id, ecs::GetFoo(e4)->health);
  printf("%u %u\n", e5->id, ecs::GetFoo(e5)->health);

  ecs::RemoveFoo(e4);

  {
    ecs::EntityItr<1> itr(kFoo);
    while (itr.Next()) {
      printf("Entity found %u %u\n", itr.eid, itr.c.foo->health);
    }
  }

  //ecs::RemoveFoo(e4);
  //ecs::RemoveFoo(e5);
#endif
  return 0;
}
