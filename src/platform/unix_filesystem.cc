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

std::string Filename(const char* fullname) {
  s32 sz = strlen(fullname);
  s32 i = sz - 1;
  while (fullname[i--] != '/');
  return std::string(fullname, i + 2, sz);
}

std::string JoinPath(const char* s1, const char* s2) {
  // TODO: Make less stupid.
  std::string ret = SanitizePath(s1);
  assert(ret.size() != 0);
  assert(strlen(s2) != 0);
  if (ret[ret.size() - 1] != '/') ret += "/";
  ret += SanitizePath(s2);
  return ret;
}

// Replaces backslashes with slashes
std::string SanitizePath(const std::string& path) {
  std::string copy = path;
  std::replace(copy.begin(), copy.end(), '\\', '/');
  return copy;
}

}  // namespace filesystem
