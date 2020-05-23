#pragma once

namespace memory {

struct Memory {
  u8* storage = nullptr;
  u64 storage_size = 0;
  u64 storage_used = 0;
};

thread_local Memory kMemory;

bool
Initialize(u64 storage_bytes)
{
  assert(kMemory.storage == nullptr);
  kMemory.storage_used = 0;
  kMemory.storage_size = storage_bytes;
  kMemory.storage = (u8*)calloc(storage_bytes, sizeof(u8));
  return kMemory.storage != nullptr;
}

u8*
PushBytes(u64 num)
{
  assert(kMemory.storage_used + num < kMemory.storage_size);
  u8* mem = &kMemory.storage[kMemory.storage_used];
  kMemory.storage_used += num;
  return mem;
}

void
PopBytes(u64 num)
{
  assert(kMemory.storage_used >= num);
  kMemory.storage_used -= num;
  memset(&kMemory.storage[kMemory.storage_used], 0, num);
}

template <typename T>
T*
PushType(u64 num)
{
  u64 sz = num * sizeof(T);
  assert(kMemory.storage_used + sz < kMemory.storage_size);
  T* mem = (T*)&kMemory.storage[kMemory.storage_used];
  kMemory.storage_used += sz;
  return mem;
}

template <typename T>
void
PopType(u64 num)
{
  u64 sz = num * sizeof(T);
  assert(kMemory.storage_used >= sz);
  kMemory.storage_used -= sz;
  memset(&kMemory.storage[kMemory.storage_used], 0, sz);
}

}  // namespace memory
