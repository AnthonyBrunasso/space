#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "platform/platform.cc"

#define FRAMERATE (60)
#define MAX_PACKET_IN (4 * 1024)
#define MAX_PLAYBACK (8 * 1024)
static uint8_t in_buffer[MAX_PACKET_IN];
static uint8_t buffer[MAX_PLAYBACK];
static uint16_t* record_start;
static uint16_t* record_end;

int
main(int argc, char** argv)
{
  uint64_t retransmit_count = 1;
  while (1) {
    int opt = platform_getopt(argc, argv, "c:");
    if (opt == -1) break;

    switch (opt) {
      case 'c':
        retransmit_count = strtol(platform_optarg, NULL, 10);
        puts("set count");
        break;
    }
  }

  if (platform_optind == argc) {
    printf("Usage: %s <record_file>\n", argv[0]);
    exit(1);
  }
  const char* filename = argv[platform_optind];
  printf("Opening %s\n", filename);
  FILE* f = fopen(filename, "r");
  if (!f) exit(2);
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

  const char* ip = "127.0.0.1";
  const char* port = "9845";

  Udp4 location;
  if (!udp::Init()) exit(1);
  if (!udp::GetAddr4(ip, port, &location)) exit(2);

  Clock_t game_clock;
  float frame_target_usec = 1000.f * 1000.f / (float)FRAMERATE;
  platform::clock_init(frame_target_usec, &game_clock);
  uint16_t* current_record = record_start;
  char* game_data = (char*)(record_end + 1);
  uint64_t send_count = 0;
  uint64_t receive_count = 0;
  while (current_record < record_end && !udp_errno) {
    uint16_t received_bytes;
    Udp4 remote_peer;
    if (udp::ReceiveAny(location, MAX_PACKET_IN, in_buffer, &received_bytes,
                        &remote_peer)) {
      receive_count += 1;
      continue;
    }
    bool send_result = true;
    for (int i = 0; i < retransmit_count; ++i) {
      send_result |= udp::Send(location, game_data, *current_record);
    }
    if (!send_result) continue;

    game_data += *current_record;
    current_record += 1;
    send_count += 1;

    const uint64_t logic_usec = platform::delta_usec(&game_clock);
    printf("[ logic_usec %lu ] [ jerk %lu ]\n", logic_usec, game_clock.jerk);
    uint64_t sleep_usec = 0;
    uint64_t sleep_count = 1;
    while (!platform::clock_sync(&game_clock, &sleep_usec)) {
      while (sleep_count) {
        --sleep_count;
        platform::sleep_usec(sleep_usec);
      };
    }
  }

  for (int i = 0; i < FRAMERATE; ++i) {
    uint16_t received_bytes;
    Udp4 remote_peer;
    if (udp::ReceiveAny(location, MAX_PACKET_IN, in_buffer, &received_bytes,
                        &remote_peer)) {
      receive_count += 1;
      continue;
    }

    uint64_t sleep_usec = 0;
    uint64_t sleep_count = 1;
    while (!platform::clock_sync(&game_clock, &sleep_usec)) {
      while (sleep_count) {
        --sleep_count;
        platform::sleep_usec(sleep_usec);
      };
    }
  }

  printf("[ Send %lu ] [ Receive %lu ] [ Expected %lu ]\n", send_count,
         receive_count, send_count * retransmit_count);

  return 0;
}
