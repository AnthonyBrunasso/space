#include <cassert>
#include <cstdio>
#include <cstdarg>

#include "platform/platform.cc"
#include "memory/memory.cc"

typedef u32 Entity;

enum TypeId {
  kFoo = 0,
  kBar = 1,
};

struct ComponentStorage {
  ComponentStorage(u32 n, u32 sz) {
    bytes = memory::PushBytes(n * sz);
    sizeof_element = sz;
    max_size = n;
  }

  u8* Assign() {
    assert(size < max_size);
    u32 idx = size * sizeof_element;
    ++size;
    return &bytes[idx];
  }

  u8* Get(u32 idx) {
    assert(idx < max_size);
    return &bytes[idx * sizeof_element];
  }

  u8* bytes = nullptr;
  u32 sizeof_element = 0;
  u32 size = 0;
  u32 max_size = 0;
};

#define DECLARE_COMPONENT(Type, tid)               \
  Type* Assign##Type(Entity ent) {                 \
    Type* t = (Type*)GetComponents(tid)->Assign(); \
    t->entity_id = ent;                            \
    return t;                                      \
  }

struct Foo {
  u32 entity_id = 0;  

  u64 health = 0;
};


struct Bar {
  u32 entity_id = 0;

  r32 stuff = 0.f;
};

ComponentStorage*
GetComponents(TypeId tid)
{
  switch (tid) {
    case kFoo: {
      static ComponentStorage f(2, sizeof(Foo));
      return &f;
    } break;
    case kBar: {
      static ComponentStorage f(2, sizeof(Bar));
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

template <u32 N>
struct EntityItr {
  EntityItr(...) {
    va_list args;
    va_start(args, N);
    u32 cnt = 0;
    va_arg(args, TypeId);
    for (u32 i = 0; i < N; ++i) {
      TypeId tid = va_arg(args, TypeId);
      types[i] = tid;
      comps[i] = GetComponents(tid);
      ++cnt;
    }
    assert(cnt == N);
    va_end(args);
  } 

  void IncreaseMin() {
    u32 min = 0xFFFFFFFF;
    u32 min_idx = 0xFFFFFFFF;
    for (u32 i = 0; i < N; ++i) {
      if (idx[i] < min) {
        min = idx[i];
        min_idx = i;
      }
    }
    ++idx[min_idx];
  }

  bool Next() {
    bool match = true;
    u32 id;
    while(1) {
      u8* ptrs[N];
      for (u32 i = 0; i < N; ++i) {
        if (idx[i] >= comps[i]->max_size) return false;
        if (idx[i] >= comps[i]->size) return false;
        ptrs[i] = comps[i]->Get(idx[i]);
      }
      // First element in component structs is an entity id... Hopefully!
      id = *((u32*)(ptrs[0]));
      for (u32 i = 1; i < N; ++i) {
        u32 oid = *((u32*)(ptrs[i]));
        if (id != oid) {
          match = false;
          break;
        }
      }
      if (match) break;
      IncreaseMin();
    }

    if (match) {
      e = id;
      for (u32 i = 0; i < N; ++i) {
        // TODO: Do I need to address 32-bit vs 64-bit here?
        u64* comp = (u64*)comps[i]->Get(idx[i]);
        void* taddress = (u64*)(&c) + types[i];
        memcpy(taddress, &comp, sizeof(u64*));
        ++idx[i];
      }
      return true;
    }

    return false;
  }

  Entity e = 0;
  Components c;

  ComponentStorage* comps[N];
  TypeId types[N];
  u32 idx[N] = {};
};

int
main(int argc, char** argv)
{
  //while (!IsDebuggerPresent()) {}
  memory::Initialize(2048);

  Entity e1 = 0;
  Entity e2 = 1;

  {
    Foo* foo = AssignFoo(e1);
    foo->health = 32;
    Bar* bar = AssignBar(e1);
    bar->stuff = 2.3f;
  }

  {
    Foo* foo = AssignFoo(e2);
    foo->health = 16;
  }


  {
    EntityItr<2> itr(kFoo, kBar);
    while (itr.Next()) {
      printf("Entity found %u %u %.2f\n",
             itr.e, itr.c.foo->health, itr.c.bar->stuff);
    }
  }

  {
    EntityItr<1> itr(kFoo);
    while (itr.Next()) {
      printf("Entity found %u %u %.2f\n", itr.e, itr.c.foo->health);
    }
  }


  return 0;
}
