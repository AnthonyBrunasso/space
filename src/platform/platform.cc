#pragma once

#if _WIN32
// Windows #defines min/max. This stops it from doing so.
#define NOMINMAX
// Reduce header size.
#define WIN32_LEAN_AND_MEAN
// Disable unneeded warnings.
// Warns about using %lu to printf uint64_t
#pragma warning(disable : 4477)
#endif

#include "common/macro.h"
#include "type.cc"

#include "x64_intrin.h"

#include "clock.cc"
#include "platform_getopt.cc"
#include "window.cc"

#if _WIN32
#include "win32_filesystem.cc"
#include "win32_sleep.cc"
#include "win32_thread.cc"
#include "win32_udp.cc"
#include "win32_tcp.cc"
#else
#include "unix_filesystem.cc"
#include "unix_sleep.cc"
#include "unix_thread.cc"
#include "unix_udp.cc"
#endif

#include "affinity.cc"
