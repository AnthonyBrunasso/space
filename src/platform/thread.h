#pragma once

#include <cstdint>

struct Thread;
typedef uint64_t (*ThreadFunc)(void* arg);

struct Thread {
  uint64_t id = 0;
#ifdef _WIN32
  HANDLE handle;
#endif
  ThreadFunc func;
  void* arg = nullptr;
  uint64_t return_value;
} ti;

namespace platform
{
bool thread_create(Thread* t);
void thread_yield();
bool thread_join(Thread* t);
void thread_exit(Thread* t, uint64_t value);
}  // namespace platform
