#include "thread.h"

#include <pthread.h>
#include <sched.h>

#include <climits>
#include <cstddef>

static_assert(offsetof(ThreadInfo, id) == 0 &&
                  sizeof(ThreadInfo::id) >= sizeof(pthread_t),
              "ThreadInfo_t layout must be compatible with pthread_t");

namespace platform
{
void*
pthread_shim(void* pthread_arg)
{
  ThreadInfo* ti = (ThreadInfo*)pthread_arg;
  ti->return_value = ti->func(ti->arg);
  return NULL;
}

bool
thread_create(ThreadInfo* t)
{
  if (t->id) return false;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create((pthread_t*)t, &attr, pthread_shim, t);

  return true;
}

void
thread_yield()
{
  sched_yield();
}

bool
thread_join(ThreadInfo* t)
{
  if (!t->id) return false;

  pthread_t* pt = (pthread_t*)t;
  pthread_join(*pt, 0);
  return true;
}

void
thread_exit(ThreadInfo* t, uint64_t value)
{
  t->return_value = value;
  pthread_exit(&t->return_value);
}

unsigned
thread_affinity_count(ThreadInfo* t)
{
  cpu_set_t mask;
  pthread_t* pt = (pthread_t*)t;
  int ret = pthread_getaffinity_np(*pt, sizeof(cpu_set_t), &mask);
  if (ret == -1) return UINT_MAX;

  return CPU_COUNT(&mask);
}

bool
thread_affinity_set(ThreadInfo* t, int cpu_index)
{
  cpu_set_t mask;
  pthread_t* pt = (pthread_t*)t;

  CPU_ZERO(&mask);
  CPU_SET(cpu_index, &mask);
  int ret = pthread_setaffinity_np(*pt, sizeof(cpu_set_t), &mask);

  return ret != -1;
}

bool
thread_affinity_clear(ThreadInfo* t, int cpu_index)
{
  cpu_set_t mask;
  pthread_t* pt = (pthread_t*)t;

  int get_ret = pthread_getaffinity_np(*pt, sizeof(cpu_set_t), &mask);
  CPU_CLR(cpu_index, &mask);
  int set_ret = pthread_setaffinity_np(*pt, sizeof(cpu_set_t), &mask);

  return set_ret != -1;
}

}  // namespace platform
