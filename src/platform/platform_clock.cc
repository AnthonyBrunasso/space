#include <cassert>
#include <cstdint>
#include <cstdio>
#include <ctime>

// TODO: This should be broken up into unix/win32_clock.

#ifdef _WIN32
#include <windows.h>
#include <profileapi.h>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#include "rdtsc.h"

EXTERN(uint64_t median_tsc_per_usec);
EXTERN(uint64_t median_usec_per_tsc);

typedef struct {
#ifdef _WIN32
  uint64_t frequency;
#endif
  // The ideal advacement cadence of tsc_clock
  uint64_t tsc_step;
  // Count the time rhythmic progression was lost
  uint64_t jerk;
  // Rhythmic time progression
  uint64_t tsc_clock;
  // Used for time delta within a single frame
  uint64_t frame_to_frame_tsc;
} TscClock_t;

#define USEC_PER_CLOCK (1000.f / 1000.f / CLOCKS_PER_SEC)
#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)
#define MAX_SAMPLES 50

// clock() will accumulate per thread, thus restricted to one
static void
__estimate_tsc_per_usec()
{
  static volatile uint64_t lock_in ALIGNAS(sizeof(uint64_t));
  static volatile uint64_t lock_out ALIGNAS(sizeof(uint64_t));
  uint64_t thread_id = lock_in++;
  while (thread_id) {
    if (lock_out) {
      return;
    }
  }

  clock_t c = clock();
  clock_t p;
  uint64_t rc = rdtsc();
  uint64_t rp;

  uint64_t tsc_per_usec[MAX_SAMPLES];
  for (int i = 0; i < MAX_SAMPLES; ++i) {
    p = c;
    rp = rc;
    do {
      // clock measures time spent in the program (NB: excludes kernel)
      // a tight loop will push the scheduler to spend time in the application
      c = clock();
    } while (c - p < CLOCKS_PER_MS);
    rc = rdtsc();

    tsc_per_usec[i] = (rc - rp) * ((c - p) * USEC_PER_CLOCK);
  }

  // insertion sort
  for (int i = 1; i < MAX_SAMPLES; ++i) {
    for (int j = i; j > 0; --j) {
      if (tsc_per_usec[j - 1] <= tsc_per_usec[j]) break;
      uint64_t tmp = tsc_per_usec[j - 1];
      tsc_per_usec[j - 1] = tsc_per_usec[j];
      tsc_per_usec[j] = tmp;
    }
  }

  ++lock_out;
  median_tsc_per_usec = tsc_per_usec[MAX_SAMPLES / 2];
}

void
__init_tsc_per_usec()
{
  if (median_tsc_per_usec) return;

  int kernel_tsc_khz = 0;
#ifdef __linux__
  FILE *f = fopen("/sys/devices/system/cpu/cpu0/tsc_freq_khz", "r");
  if (f) {
    const int MAX_BUF = 12;
    char buf[MAX_BUF];
    size_t bytes = fread(buf, 1, MAX_BUF - 1, f);
    if (bytes) {
      buf[bytes] = 0;
      fclose(f);
      kernel_tsc_khz = atoi(buf) / 1000;
      printf("using tsc_freq_khz %d\n", kernel_tsc_khz);
    }
  }
#elif __APPLE__
  uint64_t mac_kernel_tsc;
  size_t sysctl_len = sizeof(mac_kernel_tsc);
  int ret = sysctlbyname("machdep.tsc.frequency", &mac_kernel_tsc, &sysctl_len, NULL, 0);
  if (ret < 0) {
    puts("This mac does not support hardware tsc?");
    exit(5);
  }
  kernel_tsc_khz = mac_kernel_tsc/1e6;
  printf("%d kernel_tsc_khz\n", kernel_tsc_khz);
#endif

  if (kernel_tsc_khz) {
    median_tsc_per_usec = kernel_tsc_khz;
  } else {
    // kernel tsc not available - perform dynamic calculation
    __estimate_tsc_per_usec();
  }
  // inverse
  const uint64_t numerator = 1ull << 33ull;
  median_usec_per_tsc = numerator / median_tsc_per_usec;
}

static uint64_t
__tscdelta_to_usec(const TscClock_t *clock, uint64_t delta_tsc)
{
#ifdef _WIN32
  return (delta_tsc * 1e6) / clock->frequency;
#else
  return (delta_tsc * median_usec_per_tsc) >> 33ull;
#endif
}

void
clock_init(uint64_t frame_goal_usec, TscClock_t *out_clock)
{
#ifdef _WIN32
  LARGE_INTEGER now, freq;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&now);
  out_clock->frequency = freq.QuadPart;
  out_clock->tsc_step = (frame_goal_usec * out_clock->frequency) / 1e6;
  out_clock->jerk = 0;
  out_clock->tsc_clock = now.QuadPart;
  out_clock->frame_to_frame_tsc = now.QuadPart;
#else
  __init_tsc_per_usec();
  // Calculate the step
  out_clock->tsc_step = frame_goal_usec * median_tsc_per_usec;
  // Init to current time
  uint64_t now = rdtsc();
  out_clock->jerk = 0;
  out_clock->tsc_clock = now;
  out_clock->frame_to_frame_tsc = now;
#endif
}

uint64_t
clock_delta_usec(const TscClock_t *clock)
{
#ifdef _WIN32
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  uint64_t elapsed_micro = now.QuadPart - clock->frame_to_frame_tsc;
  elapsed_micro *= 1e6;
  return elapsed_micro / clock->frequency;
#else
  return __tscdelta_to_usec(clock, rdtsc() - clock->frame_to_frame_tsc);
#endif
}

bool
clock_sync(TscClock_t *clock, uint64_t *optional_sleep_usec)
{
#ifdef _WIN32
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  uint64_t tsc_now = now.QuadPart;
#else
  uint64_t tsc_now = rdtsc();
#endif
  uint64_t tsc_previous = clock->tsc_clock;
  uint64_t tsc_next = tsc_previous + clock->tsc_step;

  if (tsc_next - tsc_now < clock->tsc_step) {
    // no-op, busy wait
    // optional sleep time
    *optional_sleep_usec = __tscdelta_to_usec(clock, tsc_next - tsc_now);
    return false;
  } else if (tsc_now - tsc_next <= clock->tsc_step) {
    // frame slightly over goal time
    // trivial advancement continues
    // within tsc_step do not sleep
    clock->tsc_clock = tsc_next;
  } else if (tsc_now - tsc_next > clock->tsc_step) {
    // non-trivial time-jerk, due to hardware clock or massive perf issue
    // do not sleep
    clock->tsc_clock = tsc_now;
    clock->jerk += 1;
  }

  clock->frame_to_frame_tsc = tsc_now;
  return true;
}
