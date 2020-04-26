#include "platform/platform.cc"

static Mutex m;

void
EnterCriticalSection(int thread)
{
  LockGuard lock(&m);
  for (int j = 0; j < 10; ++j) {
    printf("Thread running %i %i\n", thread, j);
  }
}

uint64_t
ThreadStuff(void* arg)
{
  int* i = (int*)arg;
  EnterCriticalSection(*i);
  return 1;
}

int
main()
{
  Thread thread[10];
  int arg[10];
  platform::mutex_create(&m);
  for (int i = 0; i < 10; ++i) {
    //printf("Creating thread %i\n", i);
    Thread* info = &thread[i];
    arg[i] = i;
    info->func = ThreadStuff;
    info->arg = &arg[i];
    platform::thread_create(info);
  }
  for (int i = 0; i < 10; ++i) {
    platform::thread_join(&thread[i]);
  }
  return 1;
}
