#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "network.cc"

#define MAX_PLAYBACK 8 * 1024
static char buffer[MAX_PLAYBACK];
static uint16_t* record_start;
static uint16_t* record_end;

void
GatherInput(uint16_t* current_record, char* bytes)
{
  printf("Gather [ bytes %d ][ %lu sequence]\n", *current_record,
         kNetworkState.outgoing_sequence);
  InputBuffer* buffer = GetNextInputBuffer();
  memcpy(buffer->input_event, bytes, *current_record);
  buffer->used_input_event = (*current_record) / sizeof(PlatformEvent);
}

int
main(int argc, char** argv)
{
  if (argc < 2) {
    printf("Usage: %s <record_file>\n", argv[0]);
    exit(1);
  }
  FILE* f = fopen(argv[1], "r");
  fseek(f, 0, SEEK_END);
  long total_len = ftell(f);
  fseek(f, 0L, SEEK_SET);
  size_t read_len = fread(buffer, total_len, 1, f);
  fclose(f);

  if (!read_len) {
    puts("read fail");
    exit(2);
  }

  printf("Loaded file [ %lu bytes]\n", total_len);
  for (int i = 0; i < total_len; ++i) {
    printf("%02hhx", buffer[i]);
  }
  puts("");
  record_start = (uint16_t*)buffer;
  uint16_t* seek_record = (uint16_t*)buffer;
  while (*seek_record != 0) ++seek_record;
  printf("%lu records found\n", seek_record - record_start);
  record_end = seek_record;

  // kNetworkState.server_ip = "127.0.0.1";
  kNetworkState.server_ip = "ohio.rufe.org";
  kNetworkState.num_players = 1;

  NetworkSetup();

  TscClock_t game_clock;
  float frame_target_usec = 1000.f * 1000.f / 60.f;
  clock_init(frame_target_usec, &game_clock);
  uint16_t* current_record = record_start;
  char* game_data = (char*)(record_end + 1);
  uint64_t frame = 0;
  while (current_record < record_end) {
    GatherInput(current_record, game_data);
    game_data += *current_record;
    current_record += 1;

    NetworkEgress();
    NetworkIngress(frame);
    printf("Contiguous ready %d\n", NetworkContiguousSlotReady(frame));
    uint64_t advance = 1 + (NetworkContiguousSlotReady(frame) > 1);
    printf("advance %d\n", advance);
    for (int i = 0; i < advance; ++i) {
      uint64_t slot = NETQUEUE_SLOT(frame);
      if (SlotReady(slot)) {
        GetSlot(slot);
        ++frame;
        printf("[ frame %lu ]\n", frame);
      }
    }
    const uint64_t logic_usec = platform::delta_usec(&game_clock);
    printf("[ logic_usec %lu ]\n", logic_usec);
    uint64_t sleep_usec = 0;
    uint64_t sleep_count = 1;
    while (!clock_sync(&game_clock, &sleep_usec)) {
      while (sleep_count) {
        --sleep_count;
        platform::sleep_usec(sleep_usec);
      };
    }
  }

  printf("[ min %lu ] [ max %lu ]\n", kNetworkState.egress_min,
         kNetworkState.egress_max);

  return 0;
}
