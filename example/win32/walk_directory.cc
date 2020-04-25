#include "platform/platform.cc"

#if 0
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
int main(int argc, char** argv)
{
  WIN32_FIND_DATA ffd;
  LARGE_INTEGER filesize;
  char dir[MAX_PATH];
  size_t length_of_arg;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  DWORD dwError=0;
  // If the directory is not specified as a command-line argument,
  // print usage.

  if (argc != 2) {
    printf("\nUsage: %s <directory name>\n", argv[0]);
    return (-1);
  }

  if (length_of_arg > (MAX_PATH - 3)) {
    printf("\nDirectory path is too long.\n");
    return (-1);
  }

  printf("\nTarget directory is %s\n\n", argv[1]);

  // Prepare string for use with FindFile functions.  First, copy the
  // string to a buffer, then append '\*' to the directory name.
  strcpy(dir, argv[1]);
  strcat(dir, "\\*");

  // Find the first file in the directory.
  wchar_t wtext[MAX_PATH];
  mbstowcs(wtext, dir, strlen(dir) + 1);
  hFind = FindFirstFile(wtext, &ffd);

  if (INVALID_HANDLE_VALUE == hFind) {
    return dwError;
  } 

  // List all the files in the directory with some info about them.
  do {
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      printf("  %ls   <DIR>\n", ffd.cFileName);
    } else {
      filesize.LowPart = ffd.nFileSizeLow;
      filesize.HighPart = ffd.nFileSizeHigh;
      printf("  %ls   %lld bytes\n", ffd.cFileName, filesize.QuadPart);
    }
  }
  while (FindNextFile(hFind, &ffd) != 0);

  dwError = GetLastError();
  if (dwError != ERROR_NO_MORE_FILES) {
  }

  FindClose(hFind);
  return dwError;
}
#endif

void
FileCallback(const char* filename)
{
  printf("%s\n", filename);
}

int main(int argc, char** argv)
{
  filesystem::WalkDirectory("asset/*", FileCallback);
  return 0;
}
