#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "platform/platform.cc"

#define FRAMERATE (60)
#define MAX_PACKET_IN (4 * 1024)
#define MAX_PACKET_OUT (1 * 1024)
#define MAX_PLAYBACK (8 * 1024)
static uint8_t in_buffer[MAX_PACKET_IN];
static uint8_t out_buffer[MAX_PACKET_OUT];
static uint8_t buffer[MAX_PLAYBACK];
static uint16_t* record_start;
static uint16_t* record_end;

static uint64_t opt_duplicate;
static uint64_t opt_batch;
static uint64_t opt_magnify;
static const char* opt_filename;

void
parse_opt(int argc, char** argv)
{
  while (1) {
    int opt = platform_getopt(argc, argv, "d:b:m:");
    if (opt == -1) break;

    switch (opt) {
      case 'c': {
        // Send N copies of each packet
        opt_duplicate = strtol(platform_optarg, NULL, 10);
      } break;
      case 'b': {
        // Repeat record_file contents N times
        opt_batch = strtol(platform_optarg, NULL, 10);
      } break;
      case 'm': {
        // Each packet contains N copies of the record
        opt_magnify = strtol(platform_optarg, NULL, 10);
      } break;
    }
  }

  if (platform_optind == argc) {
    printf("Usage: %s <record_file>\n", argv[0]);
    exit(1);
  }
  opt_filename = argv[platform_optind];
}

int
main(int argc, char** argv)
{
  parse_opt(argc, argv);

  const uint64_t duplicate = opt_duplicate;
  const uint64_t batch_count = opt_batch;
  const uint64_t magnify = opt_magnify;
  const char* filename = opt_filename;

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

#if 0
  uint16_t* current_record = record_start;
  const uint8_t* bytes = (const uint8_t*)(record_end + 1);
  uint64_t identical = 0;
  while (current_record < record_end) {
    uint16_t len = *current_record;
    uint16_t next_len = *(current_record + 1);
    if (len == next_len) {
      if (memcmp(bytes, (bytes + len), len) == 0) {
        printf("Identical at %u\n", current_record - record_start);
        ++identical;
      }
    }
    bytes += len;
    current_record += 1;
  }
  printf("%lu identical game records\n", identical);
  exit(3);
#endif

  const char* ip = "ohio.rufe.org";
  //  const char* ip = "127.0.0.1";
  const char* port = "9845";

  Udp4 location;
  if (!udp::Init()) exit(1);
  if (!udp::GetAddr4(ip, port, &location)) exit(2);

  TscClock_t game_clock;
  float frame_target_usec = 1000.f * 1000.f / (float)FRAMERATE;
  clock_init(frame_target_usec, &game_clock);
  uint64_t total_send = 0;
  uint64_t total_receive = 0;
  for (int i = 0; i < batch_count; ++i) {
    uint64_t send_count = 0;
    uint64_t receive_count = 0;
    uint16_t* current_record = record_start;
    char* game_data = (char*)(record_end + 1);
    while (current_record < record_end && !udp_errno) {
      uint16_t received_bytes;
      Udp4 remote_peer;
      if (udp::ReceiveAny(location, MAX_PACKET_IN, in_buffer, &received_bytes,
                          &remote_peer)) {
        receive_count += 1;
        continue;
      }
      bool send_result = true;
      uint16_t record_len = *current_record;
      for (int j = 0; j < duplicate; ++j) {
        uint8_t* write_ptr = out_buffer;
        assert(magnify * record_len < MAX_PACKET_OUT);
        for (int k = 0; k < magnify; ++k) {
          memcpy(write_ptr, game_data, record_len);
          write_ptr += record_len;
        }
        send_result |= udp::Send(location, game_data, write_ptr - out_buffer);
        send_count += 1;
      }
      if (!send_result) continue;

      game_data += record_len;
      current_record += 1;

      const uint64_t logic_usec = platform::delta_usec(&game_clock);
      //      printf("[ logic_usec %lu ] [ jerk %lu ]\n", logic_usec,
      //      game_clock.jerk);
      uint64_t sleep_usec = 0;
      uint64_t sleep_count = 1;
      while (!clock_sync(&game_clock, &sleep_usec)) {
        while (sleep_count) {
          --sleep_count;
          platform::sleep_usec(sleep_usec);
        };
      }
    }

    if (udp_errno) printf("[ udp_errno %d ]\n", udp_errno);

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
      while (!clock_sync(&game_clock, &sleep_usec)) {
        while (sleep_count) {
          --sleep_count;
          platform::sleep_usec(sleep_usec);
        };
      }
    }

    if (send_count != receive_count)
      printf("[ Send %lu ] [ Receive %lu ]\n", send_count, receive_count);
    total_send += send_count;
    total_receive += receive_count;
  }

  printf("[ TotalSend %lu ] [ TotalReceive %lu ]\n", total_send, total_receive);

  return 0;
}
