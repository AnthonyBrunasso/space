#include "thread.h"

namespace platform
{

DWORD WINAPI
Win32ThreadFunc(LPVOID lpParam)
{
  Thread* ti = (Thread*)lpParam;
  u64 ret = ti->func(ti->arg);
  ti->return_value = ret;
  return ret;
}


u64
ThreadId()
{
  // TODO: Replace with intrinsic __readgsqword
  // https://docs.microsoft.com/en-us/cpp/intrinsics/readgsbyte-readgsdword-readgsqword-readgsword?view=vs-2019
  return (u64)GetCurrentThreadId();
}

b8
ThreadCreate(Thread* t)
{
  if (t->id) return false;
  t->handle = CreateThread(
      nullptr,          // Default security attributes.
      0,                // Default stack size.
      Win32ThreadFunc,  // Threaded function.
      t,                // Argument to thread function.
      0,                // Creation flags.
      (DWORD*)&t->id);  // Thread identifier.
  return t->handle != nullptr;
}

void
ThreadYield()
{
  SwitchToThread();
}

b8
ThreadJoin(Thread* t)
{
  WaitForSingleObject(t->handle, INFINITE);
  GetExitCodeThread(t->handle, (LPDWORD)&t->return_value);
  CloseHandle(t->handle);
  return 0;
}

void
ThreadExit(Thread* t, u64 value)
{
  t->return_value = value;
  ExitThread((DWORD)t->return_value);
}

b8
MutexCreate(Mutex* m)
{
  m->handle = CreateMutex(
      nullptr,      // Default security attributes.
      false,        // Not owned initially.
      nullptr);     // Unnamed mutex

  if (!m->handle) {
    printf("mutex_create error: %d\n", GetLastError());
    return false;
  }

  return true;
}

void
MutexLock(Mutex* m)
{
  if (WaitForSingleObject(m->handle, INFINITE) == WAIT_FAILED) {
    printf("mutex_lock error: %d\n", GetLastError());
  }
}

void
MutexUnlock(Mutex* m)
{
  if (!ReleaseMutex(m->handle)) {
    printf("mutex_unlock error: %d\n", GetLastError());
  }
}

void
MutexFree(Mutex* m)
{
  CloseHandle(m->handle);
}

}  // namespace platform
