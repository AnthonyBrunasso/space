#pragma once

static constexpr int kMaxBlackboardItems = 16;
static constexpr int kMaxBlackboardValueSize = 64;

static uint8_t kEmptyValue[kMaxBlackboardValueSize];

struct Blackboard {
  bool Set(int idx, const uint8_t* bytes, int size) {
    assert(idx < kMaxBlackboardItems);
    assert(size < kMaxBlackboardValueSize);
    memcpy(&value[idx], bytes, size);
    return true;
  }

  bool Get(int idx, uint8_t** bytes) {
    assert(idx < kMaxBlackboardItems);
    if (!Exists(idx)) return false;
    *bytes = value[idx];
    return true;
  }

  bool Exists(int idx) {
    assert(idx < kMaxBlackboardItems);
    return memcmp(&value[idx], &kEmptyValue, kMaxBlackboardValueSize) != 0;
  }

  void Remove(int idx) {
    assert(idx < kMaxBlackboardItems);
    memcpy(&value[idx], &kEmptyValue, kMaxBlackboardValueSize);
  }

  uint8_t value[kMaxBlackboardItems][kMaxBlackboardValueSize];
};

#define BB_SET(bb, idx, val) bb.Set((int)idx, (uint8_t*)&val, sizeof(val))
#define BB_GET(bb, idx, ptr) bb.Get((int)idx, (uint8_t**)&ptr)
#define BB_EXI(bb, idx) bb.Exists((int)idx)
#define BB_REM(bb, idx) bb.Remove((int)idx)
