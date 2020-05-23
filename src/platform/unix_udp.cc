#include <errno.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <cstring>

#include "udp.h"

EXTERN(int udp_errno);

static_assert(sizeof(Udp4::socket_address) >= sizeof(struct sockaddr_in),
              "Udp4::socket_address cannot contain struct sockaddr_in");

namespace udp
{
b8
Init()
{
  // derp: windows api compat
  return true;
}

b8
Bind(Udp4 location)
{
  if (bind(location.socket, (const struct sockaddr*)location.socket_address,
           sizeof(struct sockaddr_in)) != 0) {
    udp_errno = errno;
    return false;
  }

  return true;
}

b8
BindAddr(Udp4 peer, const char* host, const char* service_or_port)
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
    return false;
  }

  if (result->ai_addrlen != sizeof(struct sockaddr_in)) return false;

  if (bind(peer.socket, (const struct sockaddr*)result->ai_addr,
           result->ai_addrlen) != 0) {
    udp_errno = errno;
    return false;
  }

  freeaddrinfo(result);

  return true;
}

b8
Send(Udp4 peer, const void* buffer, u16 len)
{
  ssize_t bytes = sendto(peer.socket, buffer, len, MSG_DONTWAIT,
                         (const struct sockaddr*)peer.socket_address,
                         sizeof(struct sockaddr_in));

  if (bytes < 0) udp_errno = TERNARY(errno == EAGAIN, 0, errno);

  return bytes == len;
}

b8
SendTo(Udp4 location, Udp4 peer, const void* buffer, u16 len)
{
  ssize_t bytes = sendto(location.socket, buffer, len, MSG_DONTWAIT,
                         (const struct sockaddr*)peer.socket_address,
                         sizeof(struct sockaddr_in));

  if (bytes < 0) udp_errno = TERNARY(errno == EAGAIN, 0, errno);

  return bytes == len;
}

b8
ReceiveFrom(Udp4 peer, u16 buffer_len, u8* buffer, s16* bytes_received)
{
  // MUST initialize: remote_len is an in/out parameter
  struct sockaddr_in remote_addr;
  socklen_t remote_len = sizeof(struct sockaddr_in);

  do {
    ssize_t bytes = recvfrom(peer.socket, buffer, buffer_len, MSG_DONTWAIT,
                             (struct sockaddr*)&remote_addr, &remote_len);

    *bytes_received = bytes;
    if (bytes < 0) {
      udp_errno = TERNARY(errno == EAGAIN, 0, errno);
      return false;
    }

    if (remote_len != sizeof(struct sockaddr_in)) return false;

    // filter packets from unexpected peers
  } while (memcmp(&remote_addr, peer.socket_address,
                  sizeof(struct sockaddr_in)) != 0);

  return true;
}

b8
ReceiveAny(Udp4 location, u16 buffer_len, u8* buffer, u16* bytes_received,
           Udp4* from_peer)
{
  // MUST initialize: remote_len is an in/out parameter
  struct sockaddr_in remote_addr;
  socklen_t remote_len = sizeof(struct sockaddr_in);

  ssize_t bytes = recvfrom(location.socket, buffer, buffer_len, MSG_DONTWAIT,
                           (struct sockaddr*)&remote_addr, &remote_len);
  *bytes_received = bytes;
  if (bytes < 0) {
    udp_errno = TERNARY(errno == EAGAIN, 0, errno);
    return false;
  }

  assert(sizeof(struct sockaddr_in) == remote_len);

  from_peer->socket = -1;
  memcpy(from_peer->socket_address, &remote_addr, sizeof(struct sockaddr_in));

  return true;
}

void
PollUsec(Udp4 location, u64 usec)
{
  struct pollfd fd = {.fd = location.socket, .events = POLLIN};
  poll(&fd, 1, usec / 1000);
}

b8
SetLowDelay(Udp4 location)
{
  int low_delay = IPTOS_LOWDELAY;
  return (setsockopt(location.socket, IPPROTO_IP, IP_TOS, &low_delay,
                     sizeof(low_delay)) < 0);
}

b8
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

  if (out->socket == -1) {
    udp_errno = errno;
    return false;
  }

  SetLowDelay(*out);

  assert(result->ai_addrlen == sizeof(struct sockaddr_in));

  memcpy(out->socket_address, result->ai_addr, result->ai_addrlen);

  freeaddrinfo(result);

  return true;
}
}  // namespace udp
