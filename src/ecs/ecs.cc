#pragma once

#include <cassert>
#include <cstdio>
#include <cstdarg>

#include "common/common.cc"
#include "platform/platform.cc"
#include "memory/memory.cc"

namespace ecs {

struct Entity {
  u32 id = 0;
  u64 components_mask = 0;

  bool
  Has(u64 type_id) const
  {
    return FLAGGED(components_mask, type_id);
  }
};

DECLARE_HASH_ARRAY(Entity, ENTITY_COUNT);

struct ComponentStorage {
  ComponentStorage(u32 n, u32 sz) {
    bytes = memory::PushBytes(n * sz);
    assert(bytes != nullptr);
    sizeof_element = sz;
    max_size = n;
  }

  u8*
  Assign()
  {
    assert(size < max_size);
    u32 idx = size * sizeof_element;
    ++size;
    return &bytes[idx];
  }

  u8*
  Get(u32 idx)
  {
    assert(idx < max_size);
    return &bytes[idx * sizeof_element];
  }

  u8* bytes = nullptr;
  u32 sizeof_element = 0;
  u32 size = 0;
  u32 max_size = 0;
};

// Users must defined this function.
ComponentStorage*
GetComponents(u64 tid);

// Users must define this struct. It should be a struct where each
// member is a pointer to a component that matches the order of type
// ids.
struct Components;

#define DECLARE_COMPONENT(Type, tid)                    \
  Type* Assign##Type(ecs::Entity* ent) {                \
    Type* t = (Type*)ecs::GetComponents(tid)->Assign(); \
    t->entity_id = ent->id;                             \
    return t;                                           \
  }

template <u32 N>
struct EntityItr {
  EntityItr(...) {
    va_list args;
    va_start(args, N);
    u32 cnt = 0;
    va_arg(args, u64);
    for (u32 i = 0; i < N; ++i) {
      u64 tid = va_arg(args, u64);
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
      eid = id;
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

  u32 eid = 0;
  Components c;

  ComponentStorage* comps[N];
  u64 types[N];
  u32 idx[N] = {};
};

}
