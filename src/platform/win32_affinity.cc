
#include <climits>
#include <cstring>

namespace platform
{
unsigned
affinity_count()
{
  // TODO
  return UINT_MAX;
}

bool
affinity_set(int cpu_index)
{
  // TODO
  return false;
}

bool
affinity_clear(int cpu_index)
{
  // TODO
  return false;
}

}  // namespace platform
