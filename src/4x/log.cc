#pragma once

#include <cstdarg>

namespace fourx {

enum LogSeverity {
  INFO,
  WARN,
  ERR,
};


void
Log(const char* fmt, va_list argp)
{
  vprintf(fmt, argp);
}

}
