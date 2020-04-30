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

uint64_t thread_id();
bool thread_create(Thread* t);
void thread_yield();
bool thread_join(Thread* t);
void thread_exit(Thread* t, uint64_t value);

bool mutex_create(Mutex* m);
void mutex_lock(Mutex* m);
void mutex_unlock(Mutex* m);
void mutex_free(Mutex* m);

}  // namespace platform

struct LockGuard {
  LockGuard(Mutex* m) : mutex(m) {
    platform::mutex_lock(mutex);
  }

  ~LockGuard() {
    platform::mutex_unlock(mutex);
  }

 private:
  Mutex* mutex;
};

