#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>

static constexpr int kMaxBlackboardItems = 16;
static constexpr int kMaxBlackboardValueSize = 64;

static constexpr int kFoo = 0;
static constexpr int kTarget = 1;

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

#define BB_SET(bb, idx, val) bb.Set(idx, (uint8_t*)&val, sizeof(val))
#define BB_GET(bb, idx, ptr) bb.Get(idx, (uint8_t**)&ptr)
 
int
main()
{
  struct Foo {
    int x;
    int y;
  };

  Foo foo;
  Blackboard bb = {};

  printf("foo:%i\n", bb.Exists(kFoo));
  printf("target:%i\n", bb.Exists(kTarget));
 
  foo.x = 3;
  foo.y = 8;
  BB_SET(bb, kFoo, foo);

  printf("foo:%i\n", bb.Exists(kFoo));
  printf("target:%i\n", bb.Exists(kTarget));

  int target_id = 4;
  BB_SET(bb, kTarget, target_id);

  printf("foo:%i\n", bb.Exists(kFoo));
  printf("target:%i\n", bb.Exists(kTarget));

  Foo* foo_two;
  if (BB_GET(bb, kFoo, foo_two)) {
    printf("%i %i\n", foo_two->x, foo_two->y);
  }

  foo_two->x = 10;
  foo_two->y = 20;

  Foo* foo_three;
  if (BB_GET(bb, kFoo, foo_three)) {
    printf("%i %i\n", foo_three->x, foo_three->y);
  }

  int* target;
  if (BB_GET(bb, kTarget, target)) {
    printf("target:%i\n", *target);
  }

  bb.Remove(kFoo);

  printf("foo:%i\n", bb.Exists(kFoo));
  printf("target:%i\n", bb.Exists(kTarget));

  bb.Remove(kTarget);

  printf("foo:%i\n", bb.Exists(kFoo));
  printf("target:%i\n", bb.Exists(kTarget));

 
  return 0;
}
