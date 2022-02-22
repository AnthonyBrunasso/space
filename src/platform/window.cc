#pragma once

#if _WIN32
// For some reason windows is the only platform that doesn't expose these functions directly.
#include "platform/opengl.h"
#include "win32_window.cc"
#elif __APPLE__
#include "macosx_window.mm"
#elif __linux__
#include "x11_window.cc"
#endif
