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
static HANDLE kSemaphore;

static Work kWork[MAX_WORK];
static Work kWorkDone[MAX_WORK];

static uint32_t volatile kWorkCount;
static uint32_t volatile kWorkTaken;
static uint32_t volatile kWorkComplete;

void
PushWork(const char* str)
{
  Work* work = &kWork[kWorkCount];
  strcpy(work->str, str);
  _ReadWriteBarrier();
  // https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-memorybarrier
  // volatile references use acquire / release semantics.
  kWorkCount++;
}

uint64_t
WorkerFunc(void* arg)
{
  uint32_t id = platform::thread_id();
  uint32_t sleep = 0;
  while (1) {
    if (kWorkTaken < kWorkCount) {
      Work* work = &kWork[_InterlockedIncrement((LONG volatile*)&kWorkTaken) - 1];
      printf("Thread %u doing work %s\n", id, work->str);
      _InterlockedIncrement((LONG volatile*)&kWorkComplete);
    } else {
      WaitForSingleObjectEx(kSemaphore, INFINITE, FALSE);
    }
  }
  return 0;
}

int
main(int argc, char** argv)
{
  kSemaphore =
      CreateSemaphoreEx(0, 0, MAX_WORKERS, 0, 0, SEMAPHORE_ALL_ACCESS);

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
  PushWork("Test 9");
  PushWork("Test 10");

  platform::sleep_sec(1);

  return 0;
}
