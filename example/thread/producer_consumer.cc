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
  ReleaseSemaphore(kSemaphore, 1, 0);
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

  PushWork("Test A1");
  PushWork("Test A2");
  PushWork("Test A3");
  PushWork("Test A4");
  PushWork("Test A5");
  PushWork("Test A6");
  PushWork("Test A7");
  PushWork("Test A8");
  PushWork("Test A9");
  PushWork("Test A10");

  platform::sleep_sec(1);

  PushWork("Test B1");
  PushWork("Test B2");
  PushWork("Test B3");
  PushWork("Test B4");
  PushWork("Test B5");
  PushWork("Test B6");
  PushWork("Test B7");
  PushWork("Test B8");
  PushWork("Test B9");
  PushWork("Test B10");

  platform::sleep_sec(1);

  return 0;
}
