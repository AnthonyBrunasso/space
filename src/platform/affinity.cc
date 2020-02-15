#pragma once

#if _WIN32
#include "win32_affinity.cc"
#elif __APPLE__
#include "macosx_affinity.cc"
#elif __linux__
#include "linuxaffinity.cc"
#endif
