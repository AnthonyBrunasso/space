#pragma once

#include <cstdint>

struct ThreadInfo;
typedef uint64_t (*ThreadFunc)(void* arg);

struct ThreadInfo {
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
bool thread_create(ThreadInfo* t);
void thread_yield();
bool thread_join(ThreadInfo* t);
void thread_exit(ThreadInfo* t, uint64_t value);
}  // namespace platform
