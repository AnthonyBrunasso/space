// Main thread is single producer with multiple consumers.
// Main thread should never lock. Worker threads should sleep but be signaled
// to awaken when new work appears.
//
// My idea here is assets should be loaded asyncrhonously while never stalling
// the main thread.

#include "platform/platform.cc"

struct Work {
  char str[64];
};

#define MAX_WORKERS 4
#define MAX_WORK 64

static Thread kWorkers[MAX_WORKERS];

static Work kWork[MAX_WORK];
static uint32_t volatile kWorkCount;
static uint32_t volatile kWorkDone;

void
PushWork(const char* str)
{
  Work* work = &kWork[kWorkCount];
  strcpy(work->str, str);
  _ReadWriteBarrier();
  kWorkCount++;
}

uint64_t
WorkerFunc(void* arg)
{
  uint32_t id = platform::thread_id();
  uint32_t sleep = 0;
  while (kWorkDone < kWorkCount) {
    Work* work = &kWork[_InterlockedIncrement((LONG volatile*)&kWorkDone) - 1];
    printf("Thread %u doing work %s\n", id, work->str);
  }
  return 0;
}

int
main(int argc, char** argv)
{
  for (int i = 0; i < MAX_WORKERS; ++i) {
    Thread* thread = &kWorkers[i];
    thread->func = WorkerFunc;
    platform::thread_create(thread);
  }

  PushWork("Test 1");
  PushWork("Test 2");
  PushWork("Test 3");
  PushWork("Test 4");
  PushWork("Test 5");
  PushWork("Test 6");
  PushWork("Test 7");
  PushWork("Test 8");

  platform::sleep_sec(5);

  return 0;
}
