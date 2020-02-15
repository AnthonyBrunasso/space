
namespace platform
{
unsigned
thread_affinity_count()
{
  cpu_set_t mask;
  pthread_t pt = pthread_self();
  int ret = pthread_getaffinity_np(pt, sizeof(cpu_set_t), &mask);
  if (ret == -1) return UINT_MAX;

  return CPU_COUNT(&mask);
}

bool
thread_affinity_set(int cpu_index)
{
  cpu_set_t mask;
  pthread_t pt = pthread_self();

  CPU_ZERO(&mask);
  CPU_SET(cpu_index, &mask);
  int ret = pthread_setaffinity_np(pt, sizeof(cpu_set_t), &mask);

  return ret != -1;
}

bool
thread_affinity_clear(int cpu_index)
{
  cpu_set_t mask;
  pthread_t pt = pthread_self();

  int get_ret = pthread_getaffinity_np(pt, sizeof(cpu_set_t), &mask);
  CPU_CLR(cpu_index, &mask);
  int set_ret = pthread_setaffinity_np(pt, sizeof(cpu_set_t), &mask);

  return set_ret != -1;
}
}  // namespace platform
