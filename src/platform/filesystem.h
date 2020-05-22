#pragma once

namespace filesystem
{
typedef void FileCallback(const char*);

inline const char*
GetFilenameExtension(const char* filename)
{
  const char* dot = strrchr(filename, '.');
  if (!dot || dot == filename) return "";
  return dot + 1;
}

inline bool
HasExtension(const char* filename, const char* extension)
{
  const char* ext = GetFilenameExtension(filename);
  return strcmp(extension, ext) == 0;
}

// Example: ReplaceFilename("new_filename.txt", "foo/bar/file.txt")
// Modifies the second param to contain "foo/bar/new_filename.txt".
inline void
ReplaceFilename(const char* newfname, char* oldfname_with_dir)
{
  uint32_t idx = 0;
  for (int i = strlen(oldfname_with_dir) - 1; i > 0; --i) {
    if (oldfname_with_dir[i] == '/') {
      idx = i + 1;
      break;
    }
  }
  if (!idx) return;
  memcpy(&oldfname_with_dir[idx], newfname, strlen(newfname));
}

bool MakeDirectory(const char* name);
void WalkDirectory(const char* dir, FileCallback* file_callback);
void ChangeDirectory(const char* dir);
}
