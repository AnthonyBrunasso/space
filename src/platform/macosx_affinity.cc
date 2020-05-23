
namespace platform {
unsigned
thread_affinity_count()
{
  return false;
}

b8
thread_affinity_usecore(s32 cpu_index)
{
  return false;
}

b8
thread_affinity_avoidcore(s32 cpu_index)
{
  return false;
}
}
