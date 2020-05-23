#pragma once

#include <cstdint>

struct Thread;
typedef u64 (*ThreadFunc)(void* arg);

struct Thread {
  u64 id = 0;
#ifdef _WIN32
  HANDLE handle = 0;
#endif
  ThreadFunc func;
  void* arg = nullptr;
  u64 return_value;
} ti;

struct Mutex {
#ifdef _WIN32
  HANDLE handle = 0;
#else
  pthread_mutex_t lock;
#endif
};

namespace platform
{

u64 ThreadId();
b8 ThreadCreate(Thread* t);
void ThreadYield();
b8 ThreadJoin(Thread* t);
void ThreadExit(Thread* t, u64 value);

b8 MutexCreate(Mutex* m);
void MutexLock(Mutex* m);
void MutexUnlock(Mutex* m);
void MutexFree(Mutex* m);

}  // namespace platform

struct LockGuard {
  LockGuard(Mutex* m) : mutex(m) {
    platform::MutexLock(mutex);
  }

  ~LockGuard() {
    platform::MutexUnlock(mutex);
  }

 private:
  Mutex* mutex;
};

