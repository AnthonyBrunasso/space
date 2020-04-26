#include "platform/platform.cc"

uint64_t
ThreadStuff(void* arg)
{
  int* i = (int*)arg;
  printf("Thread created %i\n", *i);
  return 1;
}

int
main()
{
  ThreadInfo thread[10];
  int arg[10];
  for (int i = 0; i < 10; ++i) {
    //printf("Creating thread %i\n", i);
    ThreadInfo* info = &thread[i];
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
