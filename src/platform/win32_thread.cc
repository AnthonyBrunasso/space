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
      NULL,             // Default security attributes.
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
}

}  // namespace platform
