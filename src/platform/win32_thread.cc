#include "thread.h"

namespace platform
{

DWORD WINAPI
Win32ThreadFunc(LPVOID lpParam)
{
  Thread* ti = (Thread*)lpParam;
  uint64_t ret = ti->func(ti->arg);
  ti->return_value = ret;
  return ret;
}

bool
thread_create(Thread* t)
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
thread_yield()
{
  SwitchToThread();
}

bool
thread_join(Thread* t)
{
  WaitForSingleObject(t->handle, INFINITE);
  GetExitCodeThread(t->handle, (LPDWORD)&t->return_value);
  CloseHandle(t->handle);
  return 0;
}

void
thread_exit(Thread* t, uint64_t value)
{
  t->return_value = value;
  ExitThread((DWORD)t->return_value);
}

bool
mutex_create(Mutex* m)
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
mutex_lock(Mutex* m)
{
  if (WaitForSingleObject(m->handle, INFINITE) == WAIT_FAILED) {
    printf("mutex_lock error: %d\n", GetLastError());
  }
}

void
mutex_unlock(Mutex* m)
{
  if (!ReleaseMutex(m->handle)) {
    printf("mutex_unlock error: %d\n", GetLastError());
  }
}

void
mutex_free(Mutex* m)
{
  CloseHandle(m->handle);
}

}  // namespace platform
