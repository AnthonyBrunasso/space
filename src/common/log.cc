
#include "queue.cc"

#include <cstring>

#define MAX_LOGMESSAGE 64
struct LogMessage {
  char buffer[MAX_LOGMESSAGE];
};

DECLARE_QUEUE(LogMessage, 32);

void
Log(const char* logline, unsigned len)
{
  if (len > MAX_LOGMESSAGE) {
    len = MAX_LOGMESSAGE;
  }

  if (kWriteLogMessage - kReadLogMessage == kMaxLogMessage)
    kReadLogMessage += 1;

  unsigned bucket = kWriteLogMessage % kMaxLogMessage;
  memcpy(&kLogMessage[bucket], logline, len);
  kLogMessage[bucket].buffer[MAX_LOGMESSAGE - 1] = 0;
  kWriteLogMessage += 1;
}

const char*
ReadLog(int index)
{
  if (kWriteLogMessage - kReadLogMessage <= index) return 0;

  int bucket = (kReadLogMessage + index) % kMaxLogMessage;
  return kLogMessage[bucket].buffer;
}

unsigned
LogCount()
{
  return kWriteLogMessage - kReadLogMessage;
}

