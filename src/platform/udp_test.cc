#include "platform.cc"

#include <cstdio>

volatile s32 running = 1;

s32
main()
{
  Udp4 peer;
  if (!udp::GetAddr4("127.0.0.1", "5000", &peer)) {
    printf("fail getaddr4 %d\n", udp_errno);
    exit(1);
  }

  udp::BindAddr(peer, "127.0.0.1", "50000");

  const char* payload = "wheeeee";
  if (!udp::Send(peer, payload, sizeof(payload))) {
    printf("fail send %d\n", udp_errno);
    exit(1);
  }

  while (running) {
#define MAX_BUFFER 4 * 1024
    u8 buffer[MAX_BUFFER];
    s16 received_bytes;
    if (!udp::ReceiveFrom(peer, MAX_BUFFER - 1, buffer, &received_bytes)) {
      if (udp_errno) printf("udp_errno %d\n", udp_errno);
      if (udp_errno) running = 0;
      continue;
    }

    buffer[received_bytes] = 0;
    printf("%s [ %d  bytes ]\n", buffer, received_bytes);
  }

  return 0;
}
