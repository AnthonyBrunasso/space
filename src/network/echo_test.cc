#include <cstdio>
#include <cstring>

#include "platform/platform.cc"
#include "protocol.cc"

#define MAX_PACKET_IN 1024

int
main(int argc, char** argv)
{
  const char* ip = "0.0.0.0";
  const char* port = "9845";
  // network thread affinity set to anything but core 0
  if (platform::thread_affinity_count() > 1) {
    platform::thread_affinity_avoidcore(0);
    printf("Server thread may run on %d cores\n",
           platform::thread_affinity_count());
  }

  uint8_t in_buffer[MAX_PACKET_IN];
  if (!udp::Init()) {
    puts("server: fail init");
    return 1;
  }

  Udp4 location;
  if (!udp::GetAddr4(ip, port, &location)) {
    puts("server: fail GetAddr4");
    puts(ip);
    puts(port);
    return 2;
  }

  printf("Server binding %s:%s\n", ip, port);
  if (!udp::Bind(location)) {
    puts("server: fail Bind");
    return 3;
  }

  uint64_t realtime_usec = 0;
  uint64_t time_step_usec = 1 * 1000;
  Clock_t server_clock;
  platform::clock_init(time_step_usec, &server_clock);
  while (!udp_errno) {
    uint16_t received_bytes;
    Udp4 peer;

    uint64_t sleep_usec;
    if (platform::clock_sync(&server_clock, &sleep_usec)) {
      realtime_usec += time_step_usec;
      continue;
    }

    if (!udp::ReceiveAny(location, MAX_PACKET_IN, in_buffer, &received_bytes,
                         &peer)) {
      if (udp_errno) printf("Server ReceiveAny udp_errno %d\n", udp_errno);
      platform::sleep_usec(sleep_usec);
      continue;
    }

    if (!udp::SendTo(location, peer, in_buffer, received_bytes)) {
      if (udp_errno) printf("Server SendTo udp_errno %d\n", udp_errno);
      platform::sleep_usec(sleep_usec);
      continue;
    }
  }

  return 0;
}

