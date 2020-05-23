#include "filesystem.h"

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace filesystem
{

b8
MakeDirectory(const char* name)
{
  if (mkdir(name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
    return errno == EEXIST;

  return true;
}

void
WalkDirectory(const char* dir, FileCallback* file_callback)
{
  assert(dir[strlen(dir) - 1] == '/');
  struct dirent* entry;
  DIR* dirp = opendir(dir);
  if (!dirp) {
    printf("Unable to open %s\n", dir);
    return;
  }

  while ((entry = readdir(dirp)) != nullptr) {
    if (entry->d_type == DT_DIR) continue;
    char full_name[128] = {};
    strcat(full_name, dir);
    strcat(full_name, entry->d_name);
    file_callback(full_name);
  }
  closedir(dirp);
}

void
ChangeDirectory(const char* dir)
{
  chdir(dir);
}

}  // namespace filesystem
