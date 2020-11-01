#include "filesystem.h"

#include <windows.h>
#include <tchar.h>

namespace filesystem
{

b8
MakeDirectory(const char* name)
{
  return CreateDirectoryA(name, nullptr);
}

void
WalkDirectory(const char* dir, FileCallback* file_callback)
{
  u32 len = strlen(dir);
  assert(dir[len - 1] == '/');
  WIN32_FIND_DATAW ffd;
  wchar_t wtext[MAX_PATH] = {};
  mbstowcs(wtext, dir, len);
  wtext[len] = '*';

  HANDLE h_find = FindFirstFileW(wtext, &ffd);

  if (h_find == INVALID_HANDLE_VALUE) {
    printf("Dir not found %s\n", dir);
    return;
  }

  do {
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      continue;
    char full_name[MAX_PATH] = {};
    memcpy(full_name, dir, len);
    char filename[MAX_PATH] = {};
    wcstombs(filename,  ffd.cFileName, MAX_PATH);
    strcat(full_name, filename);
    file_callback(full_name);
  }
  while (FindNextFileW(h_find, &ffd) != 0);

  FindClose(h_find);
}

void
ChangeDirectory(const char* dir)
{
}

}  // namespace filesystem
