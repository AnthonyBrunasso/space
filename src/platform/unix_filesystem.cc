#include "filesystem.h"

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace filesystem
{
bool
MakeDirectory(const char* name)
{
  if (mkdir(name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
    return errno == EEXIST;

  return true;
}

void
WalkDirectory(const char* dir, FileCallback* file_callbacj)
{
  return false;
}
}  // namespace filesystem
