#include <cstdio>

#include "x64_intrin.h"

uint64_t ai_flags;

enum AiGoals {
  kPower = 0,
  kMine,
  kThrust,
  kAiGoals = 64,
};

inline uint64_t
flag(uint64_t id)
{
  return 1 << id;
}

int
main()
{
  int a = 0x1;
  int f = 0x3;
  printf("%da %df\n", a, f);
  ANDN(a, f);
  printf("ANDN %da %df\n", a, f);
  f = ANDN(a, f);
  printf("f = ANDN %da %df\n", a, f);
  printf("%lu satisfied needs, %lu highest satisfied\n", POPCNT(ai_flags),
         kAiGoals - LZCNT(ai_flags));

  uint64_t all = flag(kPower) | flag(kMine) | flag(kThrust);
  uint64_t desired_flags = ANDN(ai_flags, all);
  printf("all 0x%lx desired 0x%lx\n", all, desired_flags);
  uint64_t selected = TZCNT(desired_flags);
  printf("selected 0x%lx\n", flag(selected));
  ai_flags |= flag(selected);
  printf("%lu satisfied needs, %lu highest satisfied\n", POPCNT(ai_flags),
         kAiGoals - LZCNT(ai_flags));

  desired_flags = ANDN(ai_flags, all);
  printf("all 0x%lx desired 0x%lx\n", all, desired_flags);
  selected = TZCNT(desired_flags);
  printf("selected 0x%lx\n", flag(selected));
  ai_flags |= flag(selected);
  printf("%lu satisfied needs, %lu highest satisfied\n", POPCNT(ai_flags),
         kAiGoals - LZCNT(ai_flags));

  desired_flags = ANDN(ai_flags, all);
  printf("all 0x%lx desired 0x%lx\n", all, desired_flags);
  selected = TZCNT(desired_flags);
  printf("selected 0x%lx\n", flag(selected));
  ai_flags |= flag(selected);
  printf("%lu satisfied needs, %lu highest satisfied\n", POPCNT(ai_flags),
         kAiGoals - LZCNT(ai_flags));

  desired_flags = ANDN(ai_flags, all);
  printf("all 0x%lx desired 0x%lx\n", all, desired_flags);

  for (int i = 7; i > -1; --i) {
    printf("%d highest bit satisfied %lu bits NOT satisfied %lu\n", i,
           kAiGoals - LZCNT(i), TZCNT(i));
  }
  printf("blsr all init 0x%lx\n", all);
  all = BLSR(all);
  printf("blsr all first 0x%lx\n", all);
  all = BLSR(all);
  printf("blsr all second 0x%lx\n", all);
  all = BLSR(all);
  printf("blsr all third 0x%lx\n", all);

  return 0;
}

