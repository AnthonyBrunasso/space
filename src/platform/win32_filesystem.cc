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
  u32 len = (u32)strlen(dir);
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
  // TODO: Don't assume dir is relative...
  TCHAR current_directory[256];
  GetCurrentDirectory(256, current_directory);
  char cstr_dir[256];
  wcstombs(cstr_dir, current_directory, wcslen(current_directory) + 1);
  std::string full_path = JoinPath(cstr_dir, dir);
  std::wstring stemp = std::wstring(full_path.begin(), full_path.end());
  LPCWSTR sw = stemp.c_str();
  SetCurrentDirectory(sw);
}

std::string Filename(const char* fullname) {
  s32 sz = (s32)strlen(fullname);
  s32 i = sz - 1;
  while (fullname[i--] != '\\');
  return std::string(fullname, i + 2, sz);
}

const char* GetWorkingDirectory() {
  static char kStrWorkingDirectoryPath[256];
  static TCHAR kBinPath[256];
  GetCurrentDirectory(256, kBinPath);
  size_t n = 0;
  wcstombs_s(&n, kStrWorkingDirectoryPath, 256, kBinPath, 256);
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
