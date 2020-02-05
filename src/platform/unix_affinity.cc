
#include <climits>
#include <cstring>

#include <sched.h>
#include <sys/sysinfo.h>

namespace platform
{
unsigned
affinity_count()
{
  cpu_set_t mask;
  int ret = sched_getaffinity(0, sizeof(mask), &mask);
  if (ret == -1) return UINT_MAX;

  return CPU_COUNT(&mask);
}

bool
affinity_set(int cpu_index)
{
  cpu_set_t mask;

  CPU_ZERO(&mask);
  CPU_SET(cpu_index, &mask);

  int ret = sched_setaffinity(0, sizeof(mask), &mask);

  return ret != -1;
}

bool
affinity_clear(int cpu_index)
{
  cpu_set_t mask;

  int get_ret = sched_getaffinity(0, sizeof(mask), &mask);
  CPU_CLR(cpu_index, &mask);
  int set_ret = sched_setaffinity(0, sizeof(mask), &mask);

  return set_ret != -1;
}

}  // namespace platform
