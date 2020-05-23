#include <cassert>
#include <cstdio>

#include "queue.cc"

struct Command {
  u64 action;
};

struct Packet {
  u64 turn;
  char buffer[8];
};

// declare 1 game queue
DECLARE_QUEUE(Command, 8);
// declare 1+1 game queue
DECLARE_QUEUE(Packet, 8);

s32
main()
{
  // Add kMax+1 to test failed push
  for (u64 i = 0; i < kMaxCommand + 1; ++i) {
    u64 before = kWriteCommand;
    PushCommand(Command{i});
    u64 after = kWriteCommand;
    printf("Push Command %lu, index %lu->%lu\n", i, before, after);
  }

  // Pop kMax+1 to test failed pop
  for (u64 i = 0; i < kMaxCommand + 1; ++i) {
    u64 before = kReadCommand;
    Command c = PopCommand();
    u64 after = kReadCommand;
    printf("Pop Command, action %lu, index %lu->%lu\n", c.action, before,
           after);
  }

  // Many game iterations of queueing
  for (u64 overflows = 0; overflows < 4; ++overflows) {
    constexpr u64 limit = UINT64_MAX / kMaxCommand;
    printf("performing %lu iterations\n", limit);
    for (u64 i = 0; i < limit + 1; ++i) {
      for (u64 j = 0; j < kMaxCommand; ++j) {
        PushCommand(Command{j + 1});
      }
      for (u64 j = 0; j < kMaxCommand; ++j) {
        assert(PopCommand().action == j + 1);
      }
    }

    printf("%lu final writes, %lu final reads (expect unsigned overflow)\n",
           kWriteCommand, kReadCommand);
  }

  PushCommand(Command{42});
  assert(PopCommand().action == 42);
  PushCommand(Command{1});
  assert(PopCommand().action == 1);

  return 0;
}
