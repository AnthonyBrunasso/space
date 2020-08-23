#include <cstdio>

#include <platform/platform.cc>

int
main(int argc, char** argv)
{
  Tcp4 sock;

  tcp::Init();

  tcp::Connect("www.codeacademy.com", "80", &sock);

  const char* msg = "GET / HTTP/1.1\r\nHost: www.codeacademy.com\r\n\r\n";

  printf("tcp send: %i\n", tcp::Send(sock, msg, strlen(msg)));

  char res[2048] = {};

  u32 bytes_received = 0;
  tcp::ReceiveFrom(sock, 2048, res, &bytes_received);

  //printf("Got %u bytes\n", bytes_received);

  printf("tcp errno: %i\n", tcp_errno);

  return 0;
}
