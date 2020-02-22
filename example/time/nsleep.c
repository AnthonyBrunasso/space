#include <signal.h>

#include <stdio.h>
#include <time.h>

void
sh(int signal)
{
  printf("Ignoring signal %d\n", signal);
}

int
main()
{
  void* was = (void*)signal(SIGUSR1, sh);
  int seconds = 60;
  long nano = 0;
  struct timespec duration = {seconds, nano};
  struct timespec rem;
  int interrupted = nanosleep(&duration, &rem);
  printf("\nnanosleep result: %d\n", interrupted);
  while (interrupted) {
    duration = rem;
    printf("resuming nanosleep %ld %ld\n", rem.tv_sec, rem.tv_nsec);
    int interrupted = nanosleep(&duration, &rem);
    printf("\nnanosleep result: %d\n", interrupted);
  }

  return 0;
}
