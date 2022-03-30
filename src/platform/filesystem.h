#pragma once

#include <functional>

namespace filesystem
{

typedef void FileCallback(const char*, bool);

inline const char* GetFilenameExtension(const char* filename) {
  const char* dot = strrchr(filename, '.');
  if (!dot || dot == filename) return "";
  return dot + 1;
}

inline b8 HasExtension(const char* filename, const char* extension) {
  const char* ext = GetFilenameExtension(filename);
  return strcmp(extension, ext) == 0;
}

// Example: ReplaceFilename("new_filename.txt", "foo/bar/file.txt")
// Modifies the second param to contain "foo/bar/new_filename.txt".
inline void ReplaceFilename(const char* newfname, char* oldfname_with_dir) {
  u32 idx = 0;
  for (s32 i = (s32)strlen(oldfname_with_dir) - 1; i > 0; --i) {
    if (oldfname_with_dir[i] == '/') {
      idx = i + 1;
      break;
    }
  }
  if (!idx) return;
  memcpy(&oldfname_with_dir[idx], newfname, strlen(newfname));
}

std::string Filename(const char* fullname);

std::string Filename(const std::string& fullname) {
  return Filename(fullname.c_str());
}

std::string JoinPath(const char* s1, const char* s2);

std::string JoinPath(const char* s1, const std::string& s2) {
  return JoinPath(s1, s2.c_str());
}

std::string JoinPath(const std::string& s1, const char* s2) {
  return JoinPath(s1.c_str(), s2);
}

std::string SanitizePath(const std::string& path);

b8 MakeDirectory(const char* name);
void WalkDirectory(const char* dir, const std::function<void(const char*, bool)> file_callback);
void WalkDirectory(const std::string& dir, const std::function<void(const char*, bool)> file_callback) {
  WalkDirectory(dir.c_str(), file_callback);
}
void ChangeDirectory(const char* dir);
const char* GetWorkingDirectory();

// Check if the current working directory contains a specific directory.
inline b8 WorkingDirectoryContains(const char* dir) {
  b8 contains = false;
#ifdef _WIN32
  WalkDirectory(JoinPath(GetWorkingDirectory(), "*"), [&](const char* name, bool is_dir) {
#else
  WalkDirectory(JoinPath(GetWorkingDirectory(), "/"), [&](const char* name, bool is_dir) {
#endif
    if (is_dir && strcmp(dir, name) == 0) {
      contains = true;
    }
  });
  return contains;
}

}
