#include <cstdint>
#include <cstdio>

#include <sys/time.h>

#include "platform/rdtsc.h"
#include "platform/clock.cc"

#define N 50000

int
main()
{
  platform::Clock timer;
  platform::ClockStart(&timer);
  for (int i = 0; i < N; ++i) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
  }
  printf("clock_gettime(MONOTONIC): %fus\n", (double)platform::ClockEnd(&timer) / N);

  platform::ClockStart(&timer);
  for (int i = 0; i < N; ++i) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  }
  printf("clock_gettime(MONOTONIC_RAW): %fus\n", (double)platform::ClockEnd(&timer) / N);


  platform::ClockStart(&timer);
  for (int i = 0; i < N; ++i) {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
  }
  printf("clock_gettime(REALTIME): %fus\n", (double)platform::ClockEnd(&timer) / N);

  platform::ClockStart(&timer);
  for (int i = 0; i < N; ++i) {
    time(NULL);
  }
  printf("time(): %fus\n", (double)platform::ClockEnd(&timer) / N);

  platform::ClockStart(&timer);
  for (int i = 0; i < N; ++i) {
    struct timeval t;
    gettimeofday(&t, NULL);
  }
  printf("gettimeofday(): %fus\n", (double)platform::ClockEnd(&timer) / N);

  platform::ClockStart(&timer);
  for (int i = 0; i < N; ++i) {
    rdtsc();
  }
  printf("rdtsc(): %fus\n", (double)platform::ClockEnd(&timer) / N);


  return 0;
}
