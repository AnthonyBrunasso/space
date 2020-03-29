#include <cstdint>
#include <cstdio>

#include "platform.cc"

int
main(int argc, char** argv)
{
  uint64_t framerate = 30;
  int runtime_seconds = 1;
  bool yield_on_idle = false;

  while (1) {
    int opt = platform_getopt(argc, argv, "f:s:y:");

    if (opt == -1) break;

    switch (opt) {
      case 'f':
        framerate = atoi(platform_optarg);
        break;
      case 's':
        runtime_seconds = atoi(platform_optarg);
        break;
      case 'y':
        yield_on_idle = true;
        break;
    }
  }

  int target_usec = 1000.0 * 1000.0 / framerate;
  TscClock_t clock;
  uint64_t sleep_usec = 0;
  uint64_t frame = 0;
  uint64_t did_sleep = 0;
  uint64_t* delta =
      (uint64_t*)malloc(sizeof(uint64_t) * framerate * runtime_seconds);
  uint64_t* sleep_duration =
      (uint64_t*)malloc(sizeof(uint64_t) * framerate * runtime_seconds);
  memset(sleep_duration, 0, sizeof(uint64_t) * framerate * runtime_seconds);

  clock_init(target_usec, &clock);

  while (frame < framerate * runtime_seconds) {
    delta[frame] = clock_delta_usec(&clock);

    uint64_t sleep_count = yield_on_idle;
    while (!clock_sync(&clock, &sleep_usec)) {
      while (sleep_count) {
        --sleep_count;
        ++did_sleep;
        platform::sleep_usec(sleep_usec);
        sleep_duration[frame] = sleep_usec;
      }
    }
    ++frame;
  }

  puts("frame delta usec:");
  for (int i = 0; i < frame; ++i) {
    printf("%lu ", delta[i]);
  }
  puts("");

  puts("sleep duration usec:");
  for (int i = 0; i < frame; ++i) {
    printf("%lu ", sleep_duration[i]);
  }
  puts("");

  printf("[0x%lx tsc_clock] [ %lu jerk ] [ %lu frame ] [ %lu did_sleep ]\n",
         clock.tsc_clock, clock.jerk, frame, did_sleep);

  free(delta);
  free(sleep_duration);

  return 0;
}
