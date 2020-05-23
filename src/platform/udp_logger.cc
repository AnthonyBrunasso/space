#include "platform.cc"

#include <cstdio>

#define MAX_BUFFER 4 * 1024
#define IPV4_HEADER 32
#define MAX_TURN (MAX_BUFFER / 16)
#define MAX_RECEIVE (1500 - IPV4_HEADER)
#define MIN_RECEIVE 16
u8 buffer[MAX_BUFFER];
u16 record[MAX_TURN + 1];

s32
main()
{
  Udp4 local_socket;
  const char* port = "10060";
  if (!udp::GetAddr4("127.0.0.1", port, &local_socket)) {
    puts("GetAddr4 failed");
    exit(1);
  }

  if (!udp::Bind(local_socket)) {
    puts("Bind failed");
    exit(2);
  }

  printf("Listening on %s\n", port);
  printf("Limits [ %lu turns ] [ %lu bytes ]\n", MAX_TURN, MAX_BUFFER);
  TscClock_t server_clock;
  u64 time_step_usec = 1000;
  clock_init(time_step_usec, &server_clock);

  u8* write_buffer = buffer;
  u16* write_record = record;
  u64 running = 1;
  while (running) {
    if (write_buffer - buffer > (MAX_BUFFER - MAX_RECEIVE)) break;
    if (write_record - record >= MAX_TURN) break;
    Udp4 peer;
    u16 bytes = 0;

    u64 sleep_usec;
    clock_sync(&server_clock, &sleep_usec);

    if (!udp::ReceiveAny(local_socket, MAX_RECEIVE, write_buffer, &bytes,
                         &peer)) {
      if (udp_errno) running = false;
      if (udp_errno) printf("udp_errno %d\n", udp_errno);
      platform::sleep_usec(sleep_usec);
      continue;
    }

    for (s32 i = 0; i < bytes; ++i) {
      printf("%02hhx", write_buffer[i]);
    }
    puts("");

    // Save receive size
    *write_record = bytes;
    // Advance
    write_buffer += bytes;
    write_record += 1;
  }

  // Write a terminating zero
  *write_record = 0;

  printf("Written [ %lu turns ] [ %lu bytes ]\n", write_record - record,
         write_buffer - buffer);

  FILE* f = fopen("logger.bin", "w");
  write_record += 1;
  fwrite(record, sizeof(u16), write_record - record, f);
  fwrite(buffer, write_buffer - buffer, 1, f);
  fclose(f);

  return 0;
}
