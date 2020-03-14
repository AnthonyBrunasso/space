#include <cstdint>
#include <cstdio>

#include "platform/platform.cc"

#define CIRCULAR 1

struct Command {
  uint64_t action_id;
  char reserved[1024];
};

constexpr uint64_t MAX_COMMAND = 1024;
static Command queue[MAX_COMMAND] ALIGNAS(4096);
static uint64_t queue_write = 0;
static uint64_t queue_read = 0;

void
test_circular()
{
  while (queue_write < MAX_COMMAND) {
    queue[queue_write] = Command{queue_write};
    queue_write %= MAX_COMMAND;
    queue_write += 1;
    printf("queue_write slot %lu\n", queue_write);
  }
}

uint64_t
pop_circular()
{
  if (queue_write - queue_read == 0) return 0;

  uint64_t ret = queue[queue_read].action_id;
  queue_read += 1;
  queue_read %= MAX_COMMAND;

  return ret;
}

static Command slide_array[MAX_COMMAND] ALIGNAS(4096);
static uint64_t used;

void
test_compaction()
{
  for (uint64_t i = 0; i < MAX_COMMAND; ++i) {
    slide_array[i] = Command{i};
    printf("queue_write slot %lu: value %lu\n", i, slide_array[i].action_id);
    ++used;
  }
}

uint64_t
pop_and_compact()
{
  if (!used) return 0;
  uint64_t ret = slide_array[0].action_id;
  for (uint64_t j = 0; j < used - 1; ++j) {
    slide_array[j] = slide_array[j + 1];
  }
  --used;
  return ret;
}

int
main()
{
#if CIRCULAR
  puts("using circular");
  test_circular();
#else
  puts("using compaction");
  test_compaction();
#endif

  uint64_t sleep;
  for (int i = 0; i < MAX_COMMAND; ++i) {
#if CIRCULAR
    printf("%lu\n", pop_circular());
#else
    printf("%lu\n", pop_and_compact());
#endif
  }

  return 0;
}
