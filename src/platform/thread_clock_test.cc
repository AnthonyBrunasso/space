#include <cstdio>

#include "platform/platform.cc"

void
clock_loop()
{
  TscClock_t stack_clock;
  clock_init(10 * 1000, &stack_clock);

  struct timespec startts = {};
  timespec_get(&startts, TIME_UTC);
  clock_t start = clock();
  uint64_t sleep_usec;
  for (int i = 0; i < 100; ++i) {
    while (!clock_sync(&stack_clock, &sleep_usec)) {
    }
  }
  struct timespec endts = {};
  timespec_get(&endts, TIME_UTC);
  printf("%lu timespec elapsed sec\n", endts.tv_sec - startts.tv_sec);
  clock_t end = clock();
  uint64_t elapsed_ms = end - start / (CLOCKS_PER_SEC / 1000);
  printf("%lu clock elapsed\n", elapsed_ms);
}

uint64_t
thread_main(void* t)
{
  printf("thread arg %p\n", t);

  platform::thread_affinity_avoidcore(0);
  printf("Thread may run on %d cores\n", platform::thread_affinity_count());

  clock_loop();

  return 0;
}

int
main(int argc, char** argv)
{
  static ThreadInfo t;
  t.func = thread_main;
  t.arg = argv;
  platform::thread_create(&t);

  platform::thread_affinity_usecore(0);
  printf("Main may run on %d cores\n", platform::thread_affinity_count());

  clock_loop();

  platform::thread_join(&t);

  return 0;
}
