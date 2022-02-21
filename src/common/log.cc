#pragma once

enum LogLevel {
  INFO,
  WARN,
  ERR,
};

#if _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define LOG(lvl, ...)                         \
{                                             \
  switch (lvl) {                              \
    case INFO: {                              \
      printf("[INFO] ");                      \
    } break;                                  \
    case WARN: {                              \
      printf("[WARN] ");                      \
    } break;                                  \
    case ERR: {                               \
      printf("[ERROR] ");                     \
    } break;                                  \
  }                                           \
  printf("%s(%i): ", __FILENAME__, __LINE__); \
  printf(__VA_ARGS__);                        \
  printf("\n");                               \
}

#define ASSERT(cond, ...)    \
  if (!(cond))               \
    LOG(ERR, __VA_ARGS__); \
  assert((cond)); 
