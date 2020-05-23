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

u64
ThreadId()
{
  pthread_t ptid = pthread_self();
  u64 tid;
  memcpy(&tid, &ptid, MIN(sizeof(tid), sizeof(ptid)));
  return tid;
}

b8
ThreadCreate(Thread* t)
{
  if (t->id) return false;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create((pthread_t*)t, &attr, pthread_shim, t);

  return true;
}

void
ThreadYield()
{
  sched_yield();
}

b8
ThreadJoin(Thread* t)
{
  if (!t->id) return false;

  pthread_t* pt = (pthread_t*)t;
  pthread_join(*pt, 0);
  return true;
}

void
ThreadExit(Thread* t, u64 value)
{
  t->return_value = value;
  pthread_exit(&t->return_value);
}


b8
MutexCreate(Mutex* m)
{
  int res = pthread_mutex_init(&m->lock, nullptr);
  if (res) {
    printf("mutex_create error: %d\n", res);
    return false;
  }
  return true;
}

void
MutexLock(Mutex* m)
{
  int res = pthread_mutex_lock(&m->lock);
  if (res) {
    printf("mutex_lock error: %d\n", res);
  }
}

void
MutexUnlock(Mutex* m)
{
  int res = pthread_mutex_unlock(&m->lock);
  if (res) {
    printf("mutex_unlock error: %d\n", res);
  }
}

void
MutexFree(Mutex* m)
{
  pthread_mutex_destroy(&m->lock);
}

}  // namespace platform
