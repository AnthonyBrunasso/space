
namespace platform {

unsigned thread_affinity_count() { SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  return system_info.dwNumberOfProcessors;
}

b8 thread_affinity_usecore(s32 cpu_index) {
  return false;
}

b8 thread_affinity_avoidcore(s32 cpu_index) {
  // TODO
  return false;
}

}
