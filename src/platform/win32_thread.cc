#include "thread.h"

// TODO: This only works for a single thread. The API is hard
// to make consistent with pthread for the following reasons -
//
// CreateThread returns a Handle to a thread. This is a win32
// specific object. The thread API assumes an id that the
// user can pick.
//
// The functor returns a DWORD and not a void*.

namespace platform {

struct Thread {
  HANDLE handle;
  DWORD thread_id;
};

static Thread kThread;

DWORD WINAPI Win32ThreadFunc( LPVOID lpParam )
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

  kThread.handle = CreateThread(
      NULL,
      0/* Default stack size */,
      Win32ThreadFunc,
      t,
      0,
      &kThread.thread_id);

  t->id = kThread.thread_id;

  return true;
}

void
thread_yield()
{
}

bool
thread_join(ThreadInfo* t)
{
  WaitForSingleObject(kThread.handle, INFINITE);
  GetExitCodeThread(kThread.handle, (LPDWORD)&t->return_value);
  return 0;
}

void
thread_exit(ThreadInfo* t, uint64_t value)
{
}

unsigned
thread_affinity_count(ThreadInfo* t)
{
  // TODO
  return UINT_MAX;
}

bool
thread_affinity_set(ThreadInfo* t, int cpu_index)
{
  // TODO
}

bool
thread_affinity_clear(ThreadInfo* t, int cpu_index)
{
  // TODO
}

}
