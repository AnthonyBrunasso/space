#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define USEC_PER_CLOCK (1000.f / 1000.f / CLOCKS_PER_SEC)
#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)

#if defined(__i386__) || defined(__x86_64__)

u64
rdtsc(void)
{
  return __builtin_ia32_rdtsc();
}

#else

#error "rdtsc() undefined for hardware"

#endif

int
cmp(const void* lhs, const void* rhs)
{
  u64 lhv = *((const u64*)lhs);
  u64 rhv = *((const u64*)rhs);
  if (lhv < rhv) return -1;
  if (lhv > rhv) return 1;
  return 0;
}

int
main()
{
  printf("%lu clocks_per_sec\n", CLOCKS_PER_SEC);
  printf("%f clocks_per_usec\n", USEC_PER_CLOCK);
  printf("%lu clocks_per_ms\n", CLOCKS_PER_MS);

  clock_t c = clock();
  clock_t p;
  u64 rc = rdtsc();
  u64 rp;

#define MAX_SAMPLES 10
  u64 tsc_per_usec[MAX_SAMPLES];
  for (int i = 0; i < MAX_SAMPLES; ++i) {
    p = c;
    rp = rc;
    do {
      // clock measures time spent in the application
      // a tight loop will push the scheduler to spend time in the application
      c = clock();
    } while (c - p < (CLOCKS_PER_MS));
    rc = rdtsc();

    tsc_per_usec[i] = (rc - rp) * ((c - p) * USEC_PER_CLOCK);
    printf("%lu clock delta %lu raw tsc_per_usec %lu weighted tsc_per_usec\n",
           c - p, rc - rp, tsc_per_usec[i]);
  }

  qsort(tsc_per_usec, MAX_SAMPLES, sizeof(tsc_per_usec[0]), cmp);
  printf("median tsc_per_usec %lu\n", tsc_per_usec[MAX_SAMPLES / 2]);

  return 0;
}
