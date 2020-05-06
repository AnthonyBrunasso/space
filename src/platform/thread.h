#pragma once

#include <cstdint>

struct Thread;
typedef uint64_t (*ThreadFunc)(void* arg);

struct Thread {
  uint64_t id = 0;
#ifdef _WIN32
  HANDLE handle = 0;
#endif
  ThreadFunc func;
  void* arg = nullptr;
  uint64_t return_value;
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

uint64_t ThreadId();
bool ThreadCreate(Thread* t);
void ThreadYield();
bool ThreadJoin(Thread* t);
void ThreadExit(Thread* t, uint64_t value);

bool MutexCreate(Mutex* m);
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

