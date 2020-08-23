#pragma once

#include "net.h"

#pragma comment(lib, "ws2_32.lib")

extern "C" {
int tcp_errno;
}

namespace tcp {

b8
Init()
{
  WSADATA ws;
  return WSAStartup(MAKEWORD(2, 0), &ws) == 0;
}

b8
Send(Tcp4 location, const char* buffer, u32 len)
{
  int bytes = send(location.socket, buffer, len, 0);
  return bytes == len;
}

b8
ReceiveFrom(Tcp4 location, u32 buffer_len, char* buffer, u32* bytes_received)
{
  s32 bytes = 0;
  do {
    bytes = recv(location.socket, buffer, buffer_len, 0);
    printf("%s\n", buffer);
    *bytes_received += bytes;
    if (bytes < 0) {
      tcp_errno = WSAGetLastError();
      return false;
    }
  } while (bytes > 0);
  return true;
}

b8
Connect(const char* host, const char* service_or_port, Tcp4* out)
{
  static struct addrinfo hints;
  struct addrinfo* result = NULL;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = IPPROTO_TCP;

  if (getaddrinfo(host, service_or_port, &hints, &result) != 0) {
    tcp_errno = errno;
    out->socket = -1;
    return false;
  }

  out->socket =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
 
  tcp_errno = errno;
  if (out->socket == INVALID_SOCKET) {
    return false;
  }
  if (result->ai_addrlen > sizeof(struct sockaddr_in)) {
    return false;
  }

  memcpy(out->socket_address, result->ai_addr, result->ai_addrlen);

  int res = connect(out->socket, result->ai_addr, (int)result->ai_addrlen);

  if (res == SOCKET_ERROR) {
    closesocket(out->socket);
    tcp_errno = res;
    return false;
  }

  freeaddrinfo(result);

  return true;
}


}
