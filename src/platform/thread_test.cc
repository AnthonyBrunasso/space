#include <cstdint>
#include <cstdio>

#include "platform.cc"

uint64_t
AppThreadMain(void* t)
{
  printf("thread arg %p\n", t);

  for (int i = 0; i < 1000; ++i) {
    printf("thread fun ");
  }

  return 500;
}

int
main(int argc, char** argv)
{
  printf("argv %p\n", argv);

  // printf("%lu\n", sizeof(pthread_t));
  static Thread t;
  t.func = AppThreadMain;
  t.arg = argv;
  platform::thread_create(&t);

  platform::thread_join(&t);
  printf("ret %lu\n", t.return_value);
  return 0;
}
