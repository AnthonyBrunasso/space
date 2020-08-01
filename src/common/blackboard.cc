#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

constexpr s32 kMaxBlackboardItems = 16;
constexpr s32 kMaxBlackboardValueSize = 64;
constexpr u32 kMaxBlackboardCount = 64;

static u8 kEmptyValue[kMaxBlackboardValueSize];

struct Blackboard {
  bool
  Set(u64 idx, const u8* bytes, u64 size)
  {
    assert(idx < kMaxBlackboardItems);
    assert(size < kMaxBlackboardValueSize);
    memcpy(&value[idx], bytes, size);
    return true;
  }

  bool
  Get(u64 idx, const u8** bytes) const
  {
    assert(idx < kMaxBlackboardItems);
    if (!Exists(idx)) return false;
    *bytes = value[idx];
    return true;
  }

  bool
  Exists(u64 idx) const
  {
    assert(idx < kMaxBlackboardItems);
    return memcmp(&value[idx], &kEmptyValue, kMaxBlackboardValueSize) != 0;
  }

  void
  Remove(u64 idx)
  {
    assert(idx < kMaxBlackboardItems);
    memcpy(&value[idx], &kEmptyValue, kMaxBlackboardValueSize);
  }

  

  u8 value[kMaxBlackboardItems][kMaxBlackboardValueSize];
  u32 id = 0;
};

DECLARE_HASH_ARRAY(Blackboard, 64);

#define BB_SET(bb, idx, val) \
  (bb == nullptr ? false : bb->Set(idx, reinterpret_cast<const u8*>(&val), sizeof(val)))
#define BB_GET(bb, idx, ptr) \
  (bb == nullptr ? false : bb->Get(idx, reinterpret_cast<const u8**>(&ptr)))
#define BB_EXI(bb, idx) (bb == nullptr ? false : bb->Exists(idx))
#define BB_REM(bb, idx) bb->Remove(idx)
