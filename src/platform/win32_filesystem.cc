#include "filesystem.h"

#include <windows.h>
#include <tchar.h>

namespace filesystem
{

b8 MakeDirectory(const char* name) {
  return CreateDirectoryA(name, nullptr);
}

void
WalkDirectory(const char* dir, const std::function<void(const char*, bool)> file_callback)
{
  u32 len = strlen(dir);
  WIN32_FIND_DATAW ffd;
  wchar_t wtext[MAX_PATH] = {};
  mbstowcs(wtext, dir, len);

  HANDLE h_find = FindFirstFileW(wtext, &ffd);

  if (h_find == INVALID_HANDLE_VALUE) {
    LOG(INFO, "Dir not found %s", dir);
    return;
  }

  do {
    bool is_dir = ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    char full_name[MAX_PATH] = {};
    memcpy(full_name, dir, len);
    char filename[MAX_PATH] = {};
    wcstombs(filename,  ffd.cFileName, MAX_PATH);
    strcat(full_name, filename);
    file_callback(filename, is_dir);
  }
  while (FindNextFileW(h_find, &ffd) != 0);

  FindClose(h_find);
}

void ChangeDirectory(const char* dir) {
}

std::string Filename(const char* fullname) {
  s32 sz = strlen(fullname);
  s32 i = sz - 1;
  while (fullname[i--] != '\\');
  return std::string(fullname, i + 2, sz);
}

const char* GetWorkingDirectory() {
  static bool kDoOnce = true;
  static char kStrWorkingDirectoryPath[256];
  if (kDoOnce) {
    static TCHAR kBinPath[256];
    GetCurrentDirectory(256, kBinPath);
    size_t n = 0;
    wcstombs_s(&n, kStrWorkingDirectoryPath, 256, kBinPath, 256);
    kDoOnce = false;
  }
  return &kStrWorkingDirectoryPath[0];
}

std::string JoinPath(const char* s1, const char* s2) {
  // TODO: Make less stupid.
  std::string ret(s1);
  ret += "\\";
  ret += std::string(s2);
  return ret;
}



}  // namespace filesystem
