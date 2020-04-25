#pragma once

namespace filesystem
{
typedef void FileCallback(const char*);

inline const char*
GetFilenameExtension(const char* filename)
{
  const char* dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

bool MakeDirectory(const char* name);
void WalkDirectory(const char* dir, FileCallback* file_callback);
}
