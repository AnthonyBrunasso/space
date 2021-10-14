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

  b8
  Has(u64 type_id) const
  {
    return FLAGGED(components_mask, type_id);
  }
};

DECLARE_HASH_ARRAY(Entity, ENTITY_COUNT);

class ComponentStorage {
 public:
  ComponentStorage(u32 n, u32 sz, u64 tid)
  {
    bytes_ = memory::PushBytes(n * sz);
    assert(bytes_ != nullptr);
    sizeof_element_ = sz;
    max_size_ = n;
    tid_ = tid;
  }

  u8*
  Assign()
  {
    assert(size_ < max_size_);
    u32 idx = size_ * sizeof_element_;
    ++size_;
    //printf("Assign tid:%llu size:%u\n", tid_, size_);
    return &bytes_[idx];
  }

  u8*
  Get(u32 idx)
  {
    assert(idx < max_size_);
    return &bytes_[idx * sizeof_element_];
  }

  // Sorts last element in list returning the sorted element.
  u8*
  SortLastElement()
  {
    assert(size_ > 0);
    if (size_ == 1) {
      return Get(size_ - 1);
    }
    u8* ptr = Get(size_ - 1);
    u32 tid = *((u32*)ptr);
    s32 nelem = size_ - 2;
    // TODO: This probably would be faster if I found the position of where
    // this element should go, memoved everything up and put it there.
    while (nelem >= 0) {
      u8* nelem_ptr = Get(nelem);
      u32 nelem_tid = *((u32*)nelem_ptr);
      if (tid > nelem_tid) {
        return ptr;
      }
      // Swap bytes between nelem_ptr and ptr.
      // Creating the loop allows the compiler to vectorize the byte swapping
      // which ends up being faster than 3 memcpys.
      for (u32 i = 0; i < sizeof_element_; ++i) {
        u8 b = *ptr;
        *ptr = *nelem_ptr;
        *nelem_ptr = b;
        ++ptr; ++nelem_ptr;
      }
      ptr = Get(nelem);
      --nelem;
    }
    return ptr;
  }

  u8*
  Find(u32 id)
  {
    s32 l = 0;
    s32 r = size_ - 1;
    while (l <= r) {
      s32 m = l + (r - l) / 2;
      u8* bytes = Get(m);
      u32 tid = *((u32*)bytes);
      if (tid == id) return bytes;
      if (tid < id) l = m + 1;
      else r = m - 1;
    }
    return nullptr;
  }

  void
  Erase(u32 id)
  {
    if (size_ == 0) return;
    if (size_ == 1) {
      Clear();
      return;
    }
    u8* elem = Find(id);
    if (!elem) {
      //printf("ComponentStorage::Erase.Find(%u) not found\n", id);
      return;
    }
    u8* nelem = elem + sizeof_element_;
    u8* end = &bytes_[(size_ - 1) * sizeof_element_ + sizeof_element_];
    memmove(elem, elem + sizeof_element_, end - nelem);
    --size_;
    memset(Get(size_), 0, sizeof_element_);
    //printf("ComponentStorage::Erase tid:%llu size:%u\n", tid_, size_);
  }

  void
  Clear()
  {
    if (size_ == 0) return;
    memset(bytes_, 0,  sizeof_element_ * size_);
    size_ = 0;
  }

  u32
  size() const
  {
    return size_;
  }

  u32
  max_size() const
  {
    return max_size_;
  }

 private:
  u8* bytes_ = nullptr;
  u32 sizeof_element_ = 0;
  u32 size_ = 0;
  u32 max_size_ = 0;
  u64 tid_ = 0;
};

// Users must defined this function.
ComponentStorage*
GetComponents(u64 tid);

// Users must define this struct. It should be a struct where each
// member is a pointer to a component that matches the order of type
// ids.
struct Components;

void
DeleteEntity(Entity* ent, u32 max_comps = 64)
{
  // If the entity has any components left attached to it - delete them.
  if (ent->components_mask) {
    // TODO: Use find first set
    for (s32 i = 0; i < max_comps; ++i) {
      if (FLAGGED(ent->components_mask, i)) {
        //printf("GetComponents(%u)->Erase(entity:%u)\n", i, ent->id);
        GetComponents(i)->Erase(ent->id);
      }
    }
  }
  SwapAndClearEntity(ent->id);
}

#define DECLARE_COMPONENT(Type, tid)                      \
  Type* Assign##Type(ecs::Entity* ent) {                  \
    if (!ent) return nullptr;                             \
    ComponentStorage* storage = ecs::GetComponents(tid);  \
    Type* t = (Type*)storage->Assign();                   \
    *t = {};                                              \
    t->entity_id = ent->id;                               \
    SBIT(ent->components_mask, tid);                      \
    t = (Type*)storage->SortLastElement();                \
    return t;                                             \
  }                                                       \
                                                          \
  Type* Assign##Type(u32 entity_id) {                     \
    if (!entity_id) return nullptr;                       \
    return Assign##Type(ecs::FindEntity(entity_id));      \
  }                                                       \
                                                          \
  void Remove##Type(ecs::Entity* ent) {                   \
    if (!ent) return;                                     \
    ComponentStorage* storage = ecs::GetComponents(tid);  \
    storage->Erase(ent->id);                              \
    CBIT(ent->components_mask, tid);                      \
  }                                                       \
                                                          \
  Type* Get##Type(ecs::Entity* ent) {                     \
    if (!ent) return nullptr;                             \
    if (!ent->Has(tid)) return nullptr;                   \
    ComponentStorage* storage = ecs::GetComponents(tid);  \
    return (Type*)storage->Find(ent->id);                 \
  }

template <u32 N>
struct EntityItr {
  EntityItr(u32 n, ...) {
    va_list args;
    va_start(args, n);
    u32 cnt = 0;
    for (u32 i = 0; i < N; ++i) {
      u64 tid = va_arg(args, u64);
      types[i] = tid;
      comps[i] = GetComponents(tid);
      ++cnt;
    }
    assert(cnt == N);
    va_end(args);
  }

  void IncreaseMin(u8** ptrs) {
    u32 min = 0xFFFFFFFF;
    u32 min_idx = 0xFFFFFFFF;
    for (u32 i = 0; i < N; ++i) {
      u32 id = *((u32*)(ptrs[i]));
      if (id < min) {
        min = id;
        min_idx = i;
      }
    }
    ++idx[min_idx];
  }

  b8 Next() {
    b8 match = true;
    u32 id;
    while(1) {
      match = true;
      u8* ptrs[N];
      for (u32 i = 0; i < N; ++i) {
        if (idx[i] >= comps[i]->max_size()) return false;
        if (idx[i] >= comps[i]->size()) return false;
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
      IncreaseMin(ptrs);
    }

    if (match) {
      e = FindEntity(id);
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

  void Reset() {
    e = nullptr;
    c = {};
    idx = {};
  }

  Entity* e = nullptr;
  Components c;

  ComponentStorage* comps[N];
  u64 types[N];
  u32 idx[N] = {};
};

// Just some helpers to avoid typing.

#define ECS_ITR1(name, ...) \
  ecs::EntityItr<1> name(1, __VA_ARGS__);

#define ECS_ITR2(name, ...) \
  ecs::EntityItr<2> name(2, __VA_ARGS__);

#define ECS_ITR3(name, ...) \
  ecs::EntityItr<3> name(3, __VA_ARGS__);

}
