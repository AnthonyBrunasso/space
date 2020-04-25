#pragma once

namespace filesystem
{
typedef void FileCallback(const char*);

bool MakeDirectory(const char* name);
void WalkDirectory(const char* dir, FileCallback* file_callback);
}
