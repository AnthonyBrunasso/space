#include "filesystem.h"

#include <windows.h>
#include <tchar.h>

namespace filesystem
{
bool
MakeDirectory(const char* name)
{
  return CreateDirectoryA(name, nullptr);
}

void
WalkDirectory(const char* dir, FileCallback* file_callback)
{
  // Maybe I can handle the case that dir does not end with a slash or a
  // *. This function must be called with both to work
  // Example: WalkDirectory("dir/*", FileCallback)
  WIN32_FIND_DATA ffd;
  wchar_t wtext[MAX_PATH];
  mbstowcs(wtext, dir, strlen(dir) + 1);
  HANDLE h_find = FindFirstFile(wtext, &ffd);

  if (h_find == INVALID_HANDLE_VALUE) {
    printf("Dir not found %s\n", dir);
    return;
  }

  do {
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      continue;
    char full_name[MAX_PATH] = {};
    memcpy(full_name, dir, strlen(dir) - 1);
    char filename[MAX_PATH] = {};
    wcstombs(filename,  ffd.cFileName, MAX_PATH);
    strcat(full_name, filename);
    file_callback(full_name);
  }
  while (FindNextFile(h_find, &ffd) != 0);
}

}  // namespace filesystem
