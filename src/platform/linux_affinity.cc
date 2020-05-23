
namespace platform
{
unsigned
thread_affinity_count()
{
  cpu_set_t mask;
  pthread_t pt = pthread_self();
  s32 ret = pthread_getaffinity_np(pt, sizeof(cpu_set_t), &mask);
  if (ret == -1) return UINT_MAX;

  return CPU_COUNT(&mask);
}

b8
thread_affinity_usecore(s32 cpu_index)
{
  cpu_set_t mask;
  pthread_t pt = pthread_self();

  CPU_ZERO(&mask);
  CPU_SET(cpu_index, &mask);
  s32 ret = pthread_setaffinity_np(pt, sizeof(cpu_set_t), &mask);

  return ret != -1;
}

b8
thread_affinity_avoidcore(s32 cpu_index)
{
  cpu_set_t mask;
  pthread_t pt = pthread_self();

  s32 get_ret = pthread_getaffinity_np(pt, sizeof(cpu_set_t), &mask);
  CPU_CLR(cpu_index, &mask);
  s32 set_ret = pthread_setaffinity_np(pt, sizeof(cpu_set_t), &mask);

  return set_ret != -1;
}
}  // namespace platform
