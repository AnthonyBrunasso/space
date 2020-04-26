#include "thread.h"

#include "common/common.cc"

namespace platform
{

DWORD WINAPI
Win32ThreadFunc(LPVOID lpParam)
{
  ThreadInfo* ti = (ThreadInfo*)lpParam;
  uint64_t ret = ti->func(ti->arg);
  ti->return_value = ret;
  return ret;
}

bool
thread_create(ThreadInfo* t)
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
thread_join(ThreadInfo* t)
{
  WaitForSingleObject(t->handle, INFINITE);
  GetExitCodeThread(t->handle, (LPDWORD)&t->return_value);
  CloseHandle(t->handle);
  return 0;
}

void
thread_exit(ThreadInfo* t, uint64_t value)
{
}

}  // namespace platform
