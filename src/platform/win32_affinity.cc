
namespace platform {
unsigned
thread_affinity_count()
{
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  return system_info.dwNumberOfProcessors;
}

bool
thread_affinity_usecore(int cpu_index)
{
  return SetThreadAffinityMask(GetCurrentThread(), (1 << cpu_index)) != 0;
}

bool
thread_affinity_avoidcore(int cpu_index)
{
  // TODO
  return false;
}
}
