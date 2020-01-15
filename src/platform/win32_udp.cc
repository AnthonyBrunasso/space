#pragma once

#include "udp.h"

#pragma comment(lib, "ws2_32.lib")

extern "C" {
int udp_errno;
}

static_assert(sizeof(Udp4::socket_address) >= sizeof(struct sockaddr_in),
              "Udp4::socket_address cannot contain struct sockaddr_in");


namespace udp
{
bool
Init()
{
  WSADATA ws;
  return WSAStartup(MAKEWORD(2, 0), &ws) == 0;
}

bool
Bind(Udp4 location)
{
  if (bind(location.socket, (const struct sockaddr*)location.socket_address,
           sizeof(struct sockaddr_in)) != 0) {
    udp_errno = errno;
    return false;
  }

  return true;
}

bool
Send(Udp4 peer, const void* buffer, uint16_t len)
{
  int bytes = sendto(peer.socket, (const char*)buffer, len, 0,
                         (const struct sockaddr*)peer.socket_address,
                         sizeof(struct sockaddr_in));
  return bytes == len;
}

bool
SendTo(Udp4 location, Udp4 peer, const void* buffer, uint16_t len)
{
  int bytes = sendto(
      location.socket, (const char*)buffer, len, 0,
      (const struct sockaddr*)peer.socket_address, sizeof(struct sockaddr_in));

  return bytes == len;
}

bool
ReceiveFrom(Udp4 peer, uint16_t buffer_len, uint8_t* buffer,
            int16_t* bytes_received)
{
  // MUST initialize: remote_len is an in/out parameter
  struct sockaddr_in remote_addr;
  socklen_t remote_len = sizeof(struct sockaddr_in);

  do {
    int bytes = recvfrom(peer.socket, (char*)buffer, buffer_len, 0,
                         (struct sockaddr*)&remote_addr, &remote_len);

    *bytes_received = bytes;
    if (bytes < 0) {
      udp_errno = (errno == EAGAIN) ? 0 : errno;
      return false;
    }

    if (remote_len != sizeof(struct sockaddr_in)) return false;

    // filter packets from unexpected peers
  } while (memcmp(&remote_addr, peer.socket_address,
                  sizeof(struct sockaddr_in)) != 0);

  return true;
}

bool
ReceiveAny(Udp4 location, uint16_t buffer_len, uint8_t* buffer,
           uint16_t* bytes_received, Udp4* from_peer)
{
  // MUST initialize: remote_len is an in/out parameter
  struct sockaddr_in remote_addr;
  socklen_t remote_len = sizeof(struct sockaddr_in);

  int bytes = recvfrom(location.socket, (char*)buffer, buffer_len, 0,
                       (struct sockaddr*)&remote_addr, &remote_len);
  *bytes_received = bytes;
  if (bytes < 0) {
    udp_errno = (errno == EAGAIN) ? 0 : errno;
    return false;
  }

  if (sizeof(struct sockaddr_in) != remote_len) return false;

  from_peer->socket = INVALID_SOCKET;
  memcpy(from_peer->socket_address, &remote_addr, sizeof(struct sockaddr_in));

  return true;
}

bool
GetAddr4(const char* host, const char* service_or_port, Udp4* out)
{
  static struct addrinfo hints;
  struct addrinfo* result = NULL;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = 0;
  hints.ai_protocol = IPPROTO_UDP;

  if (getaddrinfo(host, service_or_port, &hints, &result) != 0) {
    udp_errno = errno;
    out->socket = -1;
    return false;
  }

  out->socket =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
 
  udp_errno = errno;
  if (out->socket == INVALID_SOCKET) return false;

  // Set non-blocking. 
  u_long mode = 1;
  int r = ioctlsocket(out->socket, FIONBIO, &mode);
  if (r != NO_ERROR) {
    udp_errno = errno;
    return false;
  }

  if (result->ai_addrlen > sizeof(struct sockaddr_in)) return false;

  memcpy(out->socket_address, result->ai_addr, result->ai_addrlen);

  freeaddrinfo(result);

  return true;
}

}  // namespace udp
