#include "platform.cc"

#include <cstdio>

volatile bool running = true;

s32
main(s32 argc, char** argv)
{
  const char* ip = "127.0.0.1";
  const char* port = "9845";

  while (1) {
    s32 opt = platform_getopt(argc, argv, "i:p:");
    if (opt == -1) break;

    switch (opt) {
      case 'i':
        ip = platform_optarg;
        break;
      case 'p':
        port = platform_optarg;
        break;
    };
  }
#define MAX_BUFFER 4 * 1024
  u8 buffer[MAX_BUFFER];
  udp::Init();

  Udp4 location;
  if (!udp::GetAddr4(ip, port, &location)) {
    puts("fail GetAddr4");
    exit(1);
  }

  printf("Server binding %s:%s\n", ip, port);
  if (!udp::Bind(location)) {
    puts("fail Bind");
    exit(1);
  }

  while (running) {
    u16 received_bytes;
    Udp4 peer;

    if (!udp::ReceiveAny(location, MAX_BUFFER, buffer, &received_bytes,
                         &peer)) {
      if (udp_errno) running = false;
      if (udp_errno) printf("udp_errno %d\n", udp_errno);
      continue;
    }

    // Echo bytes to peer
    printf("socket %d echo %d bytes\n", location.socket, received_bytes);
    if (!udp::SendTo(location, peer, buffer, received_bytes)) {
      puts("send failed");
    } else {
      puts("send ok");
    }
  }
}
