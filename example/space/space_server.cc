
#include "network/server.cc"

#include <cstdio>

int
main(int argc, char** argv)
{
  const char* ip = "0.0.0.0";
  const char* port = "9845";
  const char* num_players = "1";

  while (1) {
    int opt = platform_getopt(argc, argv, "i:p:");
    if (opt == -1) break;

    switch (opt) {
      case 'i':
        ip = platform_optarg;
        break;
      case 'p':
        port = platform_optarg;
        break;
      default:
        puts("Usage: server_server -i <ip> -p <port>");
        return 1;
    }
  }

  if (!udp::Init()) return 1;
  
  if (!CreateNetworkServer(ip, port)) return 2;

  uint64_t result = WaitForNetworkServer();
  printf("%lu\n", result);

  return 0;
}

