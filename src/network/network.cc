#pragma once

#include <cstdio>

#include "common/constants.h"
#include "math/math.cc"

#include "server.cc"

// Input events capable of being processed in one game loop
#define MAX_TICK_EVENTS 32ul
// Game loop inputs allowed in-flight on the network
#define MAX_NETQUEUE 128ul
// Convert frame id into a NETQUEUE slot
#define NETQUEUE_SLOT(sequence) ((sequence) % MAX_NETQUEUE)

struct InputBuffer {
  PlatformEvent input_event[MAX_TICK_EVENTS];
  uint64_t used_input_event = 0;
};

enum Slot {
  kSlotInFlight = 0,
  kSlotReceived,
  kSlotSimulated,
};

struct NetworkState {
  // Events handled per input game frame for NETQUEUE frames
  // History is preserved until network acknowledgement
  InputBuffer input[MAX_NETQUEUE];
  uint64_t outgoing_sequence = 1;
  // Network resources
  Udp4 socket;
  Udp4 loopback;
  uint8_t netbuffer[PAGE];
  const char* server_ip = "localhost";
  const char* server_port = "9845";
  uint64_t num_players = 1;
  // Unique id for this game
  uint64_t game_id;
  // Unique cookie for this player
  uint64_t player_cookie;
  // Unique local player id for this game
  uint64_t player_index;
  // Total players in this game
  uint64_t player_count;
  // Per Player
  InputBuffer player_input[MAX_NETQUEUE][MAX_PLAYER];
  Slot network_slot[MAX_NETQUEUE][MAX_PLAYER];
  uint64_t ack_sequence;
  uint64_t ack_frame;
  // Range of packets sent last in NetworkEgress()
  uint64_t egress_min = UINT64_MAX;
  uint64_t egress_max = 0;
};

static NetworkState kNetworkState;

bool
NetworkSetup()
{
  // No player takes action on frame 0
  // Initialize with frame 0 "ready" for update
  for (int i = 0; i < MAX_NETQUEUE; ++i) {
    Slot val = Slot(TERNARY(i == 0, kSlotReceived, kSlotSimulated));

    for (int j = 0; j < MAX_PLAYER; ++j) {
      kNetworkState.network_slot[i][j] = val;
    }
  }

  if (!udp::Init()) return false;

  if (strcmp("localhost", kNetworkState.server_ip) == 0) {
    if (!CreateNetworkServer("localhost", "9845")) return false;
  }

  if (!udp::GetAddr4(kNetworkState.server_ip, kNetworkState.server_port,
                     &kNetworkState.socket))
    return false;

  if (!udp::GetAddr4("127.0.0.1", "10060", &kNetworkState.loopback))
    return false;

  const uint64_t greeting_size = 6;
  const char greeting[greeting_size] = {"space"};
  uint64_t jerk;
  int16_t bytes_received = 0;
  Clock_t handshake_clock;
  const uint64_t usec = 5 * 1000;
  platform::clock_init(usec, &handshake_clock);
  Handshake h = {.num_players = kNetworkState.num_players};
  printf("Client: send '%s' for %lu players\n", h.greeting,
         kNetworkState.num_players);
  for (int send_count = 0; bytes_received <= 0 && send_count < 5000000;
       ++send_count) {
    if (!udp::Send(kNetworkState.socket, &h, sizeof(h))) exit(1);

    for (int per_send = 0; per_send < 10; ++per_send) {
      if (udp::ReceiveFrom(kNetworkState.socket,
                           sizeof(kNetworkState.netbuffer),
                           kNetworkState.netbuffer, &bytes_received))
        break;
      uint64_t sleep_usec = 0;
      platform::clock_sync(&handshake_clock, &sleep_usec);
      platform::sleep_usec(sleep_usec);
    }
  }

  printf("Client Handshake [bytes_received %d]\n", bytes_received);
  if (bytes_received != sizeof(NotifyGame)) exit(3);

  NotifyGame* ns = (NotifyGame*)kNetworkState.netbuffer;
  printf("Handshake success [ player_id %zu ] [ player_count %zu ] \n",
         (size_t)ns->player_id, (size_t)ns->player_count);

  kNetworkState.game_id = ns->game_id;
  kNetworkState.player_index = ns->player_id;
  kNetworkState.player_count = ns->player_count;
  kNetworkState.player_cookie = ns->cookie;

  BeginGame bg;
  bg.cookie = ns->cookie;
  bg.game_id = ns->game_id;
  if (!udp::Send(kNetworkState.socket, &bg, sizeof(bg))) exit(1);
  printf("Network request BeginGame [ game_id %zu ] [ cookie 0x%llx ]\n",
         bg.game_id, bg.cookie);

  return true;
}

InputBuffer*
GetNextInputBuffer()
{
  uint64_t slot = NETQUEUE_SLOT(kNetworkState.outgoing_sequence);

#if 1
  printf("ProcessInput [ %lu seq ][ %lu slot ]\n",
         kNetworkState.outgoing_sequence, slot);
#endif

  // If unacknowledged packets exceed the queue, give up
  if (kNetworkState.outgoing_sequence - kNetworkState.ack_sequence >=
      MAX_NETQUEUE)
    exit(2);

  for (int i = 0; i < kNetworkState.num_players; ++i) {
    assert(kNetworkState.network_slot[slot][i] == kSlotSimulated);
    kNetworkState.network_slot[slot][i] = kSlotInFlight;
  }
  kNetworkState.outgoing_sequence += 1;

  return &kNetworkState.input[slot];
}

InputBuffer*
GetSlot(uint64_t slot)
{
  for (int i = 0; i < kNetworkState.num_players; ++i) {
    assert(kNetworkState.network_slot[slot][i] == kSlotReceived);
    kNetworkState.network_slot[slot][i] = kSlotSimulated;
  }
  return kNetworkState.player_input[slot];
}

bool
SlotReady(uint64_t slot)
{
  for (int i = 0; i < kNetworkState.player_count; ++i) {
    if (kNetworkState.network_slot[slot][i] != kSlotReceived) return false;
  }

  return true;
}

uint64_t
NetworkContiguousSlotReady(uint64_t frame)
{
  uint64_t count = 0;
  for (int i = 0; i < MAX_NETQUEUE; ++i) {
    uint64_t check_slot = NETQUEUE_SLOT(frame + i);
    if (!SlotReady(check_slot)) return count;
    ++count;
  }

  return MAX_NETQUEUE;
}

void
NetworkSend(uint64_t player_index, uint64_t seq)
{
  uint64_t slot = NETQUEUE_SLOT(seq);

  if (kNetworkState.network_slot[slot][player_index] >= kSlotReceived) return;
  InputBuffer* ibuf = &kNetworkState.input[slot];

  // write frame
  Turn* header = (Turn*)kNetworkState.netbuffer;
  header->sequence = seq;
  header->player_id = kNetworkState.player_index;
  header->ack_frame = kNetworkState.ack_frame;
#if 1
  printf(
      "CliSnd [ %lu slot ] [ %lu seq ] [ %lu ack_frame ] [ %lu player_index ] "
      "[ %lu events ]\n",
      slot, seq, kNetworkState.ack_frame, kNetworkState.player_index,
      ibuf->used_input_event);
#endif

  // write input
  memcpy(header->event, ibuf->input_event,
         sizeof(PlatformEvent) * ibuf->used_input_event);

  // Failure will be retried
  udp::Send(kNetworkState.socket, kNetworkState.netbuffer,
            sizeof(Turn) + sizeof(PlatformEvent) * ibuf->used_input_event);
}

void
LoopbackCopy(uint64_t sequence)
{
  uint64_t slot = NETQUEUE_SLOT(sequence);

  InputBuffer* ibuf = &kNetworkState.input[slot];
  udp::Send(kNetworkState.loopback, ibuf->input_event,
            sizeof(PlatformEvent) * ibuf->used_input_event);
}

uint64_t
NetworkEgress()
{
  uint64_t player_index = kNetworkState.player_index;
  uint64_t begin_seq = kNetworkState.ack_sequence + 1;
  uint64_t end_seq = kNetworkState.outgoing_sequence;

  LoopbackCopy(end_seq - 1);

  // Re-send input history
  uint64_t count = 0;
  for (uint64_t i = begin_seq; i < end_seq; ++i) {
    NetworkSend(player_index, i);
    ++count;
  }

  bool received_ack = kNetworkState.ack_sequence > 0;
  uint64_t min_value = (received_ack * count) + (!received_ack * UINT64_MAX);
  uint64_t max_value = count;

  kNetworkState.egress_min = MIN(kNetworkState.egress_min, min_value);
  kNetworkState.egress_max = MAX(kNetworkState.egress_max, max_value);

  return count;
}

void
NetworkIngress(uint64_t next_simulation_frame)
{
  uint64_t local_player = kNetworkState.player_index;

  int16_t bytes_received;
  while (udp::ReceiveFrom(kNetworkState.socket, sizeof(kNetworkState.netbuffer),
                          kNetworkState.netbuffer, &bytes_received)) {
    NotifyTurn* header = (NotifyTurn*)kNetworkState.netbuffer;
    uint64_t header_frame = header->frame;
    uint64_t player_index = header->player_id;
#if 1
    printf(
        "CliRcv [ %lu next_simulation_frame ] [ %lu header_frame ] [ %lu "
        "player_index ] [ %lu ack_seq ]\n",
        next_simulation_frame, header_frame, player_index,
        header->ack_sequence);
#endif

    // Drop old frames, the game has progressed
    if (header_frame < next_simulation_frame) continue;
    // Drop out of range frames
    if (header_frame - next_simulation_frame >= MAX_NETQUEUE) continue;
    // Personal boundaries
    if (player_index >= MAX_PLAYER) exit(1);
    if (bytes_received > sizeof(NotifyTurn) + sizeof(InputBuffer::input_event))
      exit(3);

    uint64_t slot = NETQUEUE_SLOT(header_frame);
    InputBuffer* ibuf = &kNetworkState.player_input[slot][player_index];
    memcpy(ibuf->input_event, header->event,
           bytes_received - sizeof(NotifyTurn));
    ibuf->used_input_event =
        (bytes_received - sizeof(NotifyTurn)) / sizeof(PlatformEvent);
#if 0
    printf("Copied %lu, used_input_event %lu\n",
           bytes_received - sizeof(NotifyTurn), ibuf->used_input_event);
#endif
    kNetworkState.network_slot[slot][player_index] = kSlotReceived;
    // Accept highest received ack_sequence
    kNetworkState.ack_sequence =
        MAX(kNetworkState.ack_sequence, header->ack_sequence);
  }

  uint64_t ready_to_frame =
      next_simulation_frame + NetworkContiguousSlotReady(next_simulation_frame);
  kNetworkState.ack_frame = ready_to_frame - 1;
}

uint64_t
NetworkQueueGoal()
{
  // No ack data yet, allow unbuffered play
  if (kNetworkState.egress_min == UINT64_MAX) return 1;

  return MAX(kNetworkState.egress_max - kNetworkState.egress_min, 1);
}

