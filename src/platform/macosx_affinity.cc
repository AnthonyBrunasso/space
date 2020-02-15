
namespace platform {
unsigned
thread_affinity_count()
{
  return false;
}

bool
thread_affinity_set(int cpu_index)
{
  return false;
}

bool
thread_affinity_clear(int cpu_index)
{
  return false;
}
}
