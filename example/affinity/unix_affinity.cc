#include <limits.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

int
logaffinity(int cpu_count)
{
  cpu_set_t mask;
  if (sched_getaffinity(0, sizeof(mask), &mask) == -1) {
    perror("sched_getaffinity failed");
    return 1;
  }

  for (int i = 0; i < cpu_count; ++i) {
    if
      CPU_ISSET(i, &mask)
    printf("%d core enabled\n", i);
    else printf("%d core disabled\n", i);
  }

  return 0;
}

int
main()
{
  cpu_set_t defaultMask;
  CPU_ZERO(&defaultMask);
  if (sched_getaffinity(0, sizeof(defaultMask), &defaultMask) == -1) {
    perror("sched_getaffinity failed");
    return 1;
  }

  int cpu_count = get_nprocs();
  printf("%d cpus\n", cpu_count);

  logaffinity(cpu_count);

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  puts("setaffinity to core 0");
  if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
    perror("sched_setaffinity failed");
    return 1;
  }

  logaffinity(cpu_count);

  CPU_CLR(0, &mask);
  puts("setaffinity to none");
  if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
    perror("sched_setaffinity failed");
    return 1;
  }

  logaffinity(cpu_count);

  return 0;
}
