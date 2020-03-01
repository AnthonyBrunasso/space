#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

static constexpr int kMaxBlackboardItems = 16;
static constexpr int kMaxBlackboardValueSize = 64;

static uint8_t kEmptyValue[kMaxBlackboardValueSize];

struct Blackboard {
  bool
  Set(uint64_t idx, const uint8_t* bytes, uint64_t size)
  {
    assert(idx < kMaxBlackboardItems);
    assert(size < kMaxBlackboardValueSize);
    memcpy(&value[idx], bytes, size);
    return true;
  }

  bool
  Get(uint64_t idx, const uint8_t** bytes) const
  {
    assert(idx < kMaxBlackboardItems);
    if (!Exists(idx)) return false;
    *bytes = value[idx];
    return true;
  }

  bool
  Exists(uint64_t idx) const
  {
    assert(idx < kMaxBlackboardItems);
    return memcmp(&value[idx], &kEmptyValue, kMaxBlackboardValueSize) != 0;
  }

  void
  Remove(uint64_t idx)
  {
    assert(idx < kMaxBlackboardItems);
    memcpy(&value[idx], &kEmptyValue, kMaxBlackboardValueSize);
  }

  uint8_t value[kMaxBlackboardItems][kMaxBlackboardValueSize];
};

#define BB_SET(bb, idx, val) \
  bb.Set(idx, reinterpret_cast<const uint8_t*>(&val), sizeof(val))
#define BB_GET(bb, idx, ptr) \
  bb.Get(idx, reinterpret_cast<const uint8_t**>(&ptr))
#define BB_EXI(bb, idx) bb.Exists(idx)
#define BB_REM(bb, idx) bb.Remove(idx)
