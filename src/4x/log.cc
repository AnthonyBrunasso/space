#pragma once

#include <cstdarg>

enum LogSeverity {
  INFO,
  WARN,
  ERR,
};

namespace fourx {

void
LogFServer(const char* fmt, va_list argp)
{
  vprintf(fmt, argp);
}

#define LOGF(severity, fmt, ...) \
  LogFServer(fmt, __VA_ARGS__);

}
