
#include "queue.cc"

#include <cstring>

#define MAX_LOGLINE 64
struct LogMessage {
  char buffer[MAX_LOGLINE];
};

DECLARE_QUEUE(LogMessage, 32);
static char kLogBuffer[MAX_LOGLINE];
static s32 kUsedLogBuffer;

void
Log(const char* logline, unsigned len)
{
  if (kWriteLogMessage - kReadLogMessage == kMaxLogMessage)
    kReadLogMessage += 1;

  const unsigned bucket = MOD_BUCKET(kWriteLogMessage, kMaxLogMessage);
  const unsigned line_len = MIN(len, MAX_LOGLINE);
  memcpy(&kLogMessage[bucket].buffer[0], logline, line_len);
  kLogMessage[bucket].buffer[line_len] = 0;
  kWriteLogMessage += 1;
}

const char*
ReadLog(s32 index)
{
  if (kWriteLogMessage - kReadLogMessage <= index) return 0;

  const unsigned bucket = MOD_BUCKET(kReadLogMessage + index, kMaxLogMessage);
  return kLogMessage[bucket].buffer;
}

unsigned
LogCount()
{
  return kWriteLogMessage - kReadLogMessage;
}

#define LOG(x) (Log(x, strlen(x)))
#define LOGFMT(fmt, ...)                                        \
  kUsedLogBuffer = snprintf(kLogBuffer, MAX_LOGLINE, fmt, __VA_ARGS__); \
  Log(kLogBuffer, kUsedLogBuffer);
