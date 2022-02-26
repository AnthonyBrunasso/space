#include "filesystem.h"

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace filesystem
{

b8 MakeDirectory(const char* name) {
  if (mkdir(name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
    return errno == EEXIST;

  return true;
}

void WalkDirectory(const char* dir, const std::function<void(const char*, bool)> file_callback) {
  assert(dir[strlen(dir) - 1] == '/');
  struct dirent* entry;
  DIR* dirp = opendir(dir);
  if (!dirp) {
    printf("Unable to open %s\n", dir);
    return;
  }

  while ((entry = readdir(dirp)) != nullptr) {
    bool is_dir = entry->d_type == DT_DIR;
    char full_name[128] = {};
    strcat(full_name, entry->d_name);
    file_callback(full_name, is_dir);
  }

  closedir(dirp);
}

void ChangeDirectory(const char* dir) {
  chdir(dir);
}

const char* GetWorkingDirectory() {
  static char cwd[256];
  memset(cwd, 0, 256);
  getcwd(cwd, 256);
  return cwd;
}

}  // namespace filesystem
