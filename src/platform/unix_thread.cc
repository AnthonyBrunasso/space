#include "thread.h"

#include <pthread.h>
#include <sched.h>

#include <climits>
#include <cstddef>

static_assert(offsetof(Thread, id) == 0 &&
              sizeof(Thread::id) >= sizeof(pthread_t),
              "Thread_t layout must be compatible with pthread_t");

namespace platform
{
void*
pthread_shim(void* pthread_arg)
{
  Thread* ti = (Thread*)pthread_arg;
  ti->return_value = ti->func(ti->arg);
  return NULL;
}

bool
thread_create(Thread* t)
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
thread_join(Thread* t)
{
  if (!t->id) return false;

  pthread_t* pt = (pthread_t*)t;
  pthread_join(*pt, 0);
  return true;
}

void
thread_exit(Thread* t, uint64_t value)
{
  t->return_value = value;
  pthread_exit(&t->return_value);
}


bool
mutex_create(Mutex* m)
{
  int res = pthread_mutex_init(&m->lock, nullptr);
  if (res) {
    printf("mutex_create error: %d\n", res);
    return false;
  }
  return true;
}

void
mutex_lock(Mutex* m)
{
  int res = pthread_mutex_lock(&m->lock);
  if (res) {
    printf("mutex_lock error: %d\n", res);
  }
}

void
mutex_unlock(Mutex* m)
{
  int res = pthread_mutex_unlock(&m->lock);
  if (res) {
    printf("mutex_unlock error: %d\n", res);
  }
}

void
mutex_free(Mutex* m)
{
  pthread_mutex_destroy(&m->lock);
}

}  // namespace platform
