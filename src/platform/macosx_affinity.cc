
namespace platform {
unsigned
thread_affinity_count()
{
  return false;
}

bool
thread_affinity_usecore(int cpu_index)
{
  return false;
}

bool
thread_affinity_avoidcore(int cpu_index)
{
  return false;
}
}
