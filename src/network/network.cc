#pragma once

#include <cassert>
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
// Largest network message
#define MAX_NETBUFFER (PAGE)
// Update packets per frame
#define MAX_UPDATE 1

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
  uint8_t netbuffer[MAX_NETBUFFER];
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
  PlayerInfo player_info[MAX_PLAYER];
  // 1.0: 84th percentile, 2.0: 97th percentile, 3.0: 99th percentile
  const float rsdev_const = 3.0f;
  // Server state
  uint64_t server_jerk;
};

enum
{
  kNeNone = 0,
  kNeSendFail,
  kNeCorrupt,
  kNeExcessLatency,
};

static NetworkState kNetworkState;
static Stats kNetworkStats;
EXTERN(uint64_t kNetworkExit);

bool
NetworkSetup()
{
  StatsInit(&kNetworkStats);

  // Player takes no action on frame 0
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

  uint64_t jerk;
  int16_t bytes_received = 0;
  TscClock_t handshake_clock;
  const uint64_t usec = 50 * 1000;
  clock_init(usec, &handshake_clock);
  v2f dims = window::GetWindowSize();
  Handshake h;
  h.num_players = kNetworkState.num_players;
  h.player_info.window_width = (uint64_t)dims.x;
  h.player_info.window_height = (uint64_t)dims.y;
  if (ALAN) {
    printf("Client: send '%s' for %lu players client dims(%lu, %lu)\n",
           h.greeting, kNetworkState.num_players, h.player_info.window_width,
           h.player_info.window_height);
  }
  for (int send_count = 0; bytes_received <= 0 && send_count < 5000000;
       ++send_count) {
    if (!udp::Send(kNetworkState.socket, &h, sizeof(h))) {
      kNetworkExit = kNeSendFail;
      return false;
    }

    for (int per_send = 0; per_send < 10; ++per_send) {
      if (udp::ReceiveFrom(kNetworkState.socket,
                           sizeof(kNetworkState.netbuffer),
                           kNetworkState.netbuffer, &bytes_received))
        break;
      uint64_t sleep_usec = 0;
      clock_sync(&handshake_clock, &sleep_usec);
      platform::sleep_usec(sleep_usec);
    }
  }

  if (ALAN) {
    printf("Client Handshake [bytes_received %d]\n", bytes_received);
  }
  if (bytes_received != sizeof(NotifyGame)) {
    kNetworkExit = kNeCorrupt;
    return false;
  }

  NotifyGame* ns = (NotifyGame*)kNetworkState.netbuffer;
  if (ALAN) {
    printf("Handshake success [ player_id %zu ] [ player_count %zu ] [",
           (size_t)ns->player_id, (size_t)ns->player_count);
    for (int i = 0; i < ns->player_count; ++i) {
      printf(" %lu,%lu ", ns->player_info[i].window_width,
             ns->player_info[i].window_height);
    }
    printf("]\n");
  }
  kNetworkState.game_id = ns->game_id;
  kNetworkState.player_index = ns->player_id;
  kNetworkState.player_count = ns->player_count;
  kNetworkState.player_cookie = ns->cookie;
  for (int i = 0; i < ns->player_count; ++i) {
    kNetworkState.player_info[i] = ns->player_info[i];
  }

  BeginGame bg;
  bg.cookie = ns->cookie;
  bg.game_id = ns->game_id;
  if (!udp::Send(kNetworkState.socket, &bg, sizeof(bg))) {
    kNetworkExit = kNeSendFail;
    return false;
  }
  printf("Network request BeginGame [ game_id %zu ] [ cookie 0x%llx ]\n",
         bg.game_id, bg.cookie);

  return true;
}

InputBuffer*
GetNextInputBuffer()
{
  uint64_t slot = NETQUEUE_SLOT(kNetworkState.outgoing_sequence);

  if (ALAN) {
    printf("--ProcessInput [ %lu seq ][ %lu slot ]\n",
           kNetworkState.outgoing_sequence, slot);
  }

  // If unacknowledged packets exceed the queue, give up
  if (kNetworkState.outgoing_sequence - kNetworkState.ack_sequence >=
      MAX_NETQUEUE) {
    kNetworkExit = kNeExcessLatency;
    return NULL;
  }

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

bool
NetworkAppend(uint64_t player_index, const uint8_t* end_buffer,
              uint8_t** write_ref, uint64_t* seq_ref)
{
  uint64_t slot = NETQUEUE_SLOT(*seq_ref);
  uint8_t* netbuffer = *write_ref;

  if (kNetworkState.network_slot[slot][player_index] >= kSlotReceived)
    return false;

  InputBuffer* ibuf = &kNetworkState.input[slot];
  Turn* turn = (Turn*)netbuffer;
  uint8_t* event = netbuffer + sizeof(Turn);
  uint64_t event_bytes = sizeof(PlatformEvent) * ibuf->used_input_event;

  // Full packet
  if (end_buffer - netbuffer < event_bytes) return false;

  if (ALAN) {
    printf(
        "Client NetworkAppend "
        "[ event_bytes %lu ] "
        "[ sequence %lu ] "
        "\n",
        event_bytes, *seq_ref);
  }

  turn->event_bytes = event_bytes;
  memcpy(event, ibuf->input_event, event_bytes);

  // Advance
  *write_ref += event_bytes + sizeof(Turn);
  *seq_ref += 1;

  return true;
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

  if (ALAN) LoopbackCopy(end_seq - 1);

  // Re-send input history
  uint64_t seq = begin_seq;
  for (int i = 0; i < MAX_UPDATE; ++i) {
    Update* header = (Update*)kNetworkState.netbuffer;
    header->sequence = seq;
    header->ack_frame = kNetworkState.ack_frame;

    uint8_t* write_buffer = kNetworkState.netbuffer + sizeof(Update);
    const uint8_t* end_buffer =
        kNetworkState.netbuffer + sizeof(kNetworkState.netbuffer);
    while (seq < end_seq) {
      if (!NetworkAppend(player_index, end_buffer, &write_buffer, &seq)) break;
    }

    if (ALAN) {
      printf(
          "CliSnd "
          "[ %lu player_index ] "
          "[ %lu header_sequence ] "
          "[ %lu ack_sequence ] "
          "[ %lu ack_frame ] "
          "[ %ld written ] "
          "\n",
          kNetworkState.player_index, header->sequence,
          kNetworkState.ack_sequence, kNetworkState.ack_frame,
          write_buffer - kNetworkState.netbuffer);
    }

    udp::Send(kNetworkState.socket, kNetworkState.netbuffer,
              write_buffer - kNetworkState.netbuffer);
  }

  bool received_ack = kNetworkState.ack_sequence > 0;
  uint64_t count = end_seq - begin_seq;
  uint64_t min_value = (received_ack * count) + (!received_ack * UINT64_MAX);
  uint64_t max_value = count;

  kNetworkState.egress_min = MIN(kNetworkState.egress_min, min_value);
  kNetworkState.egress_max = MAX(kNetworkState.egress_max, max_value);

  if (received_ack) {
    StatsAdd(count, &kNetworkStats);
  }

  return count;
}

void
NetworkIngress(uint64_t next_simulation_frame)
{
  int16_t bytes_received;
  while (udp::ReceiveFrom(kNetworkState.socket, sizeof(kNetworkState.netbuffer),
                          kNetworkState.netbuffer, &bytes_received)) {
    NotifyUpdate* update = (NotifyUpdate*)kNetworkState.netbuffer;
    const uint64_t ack_sequence = update->ack_sequence;
    const int64_t ack_delta = ack_sequence - update->ack_sequence;

    if (ALAN) {
      printf(
          "CliRcv "
          "[ %lu ack_seq ] "
          "[ %ld ack_delta ] "
          "\n",
          ack_sequence, ack_delta);
    }

    if (ack_delta < 0) continue;
    if (ack_sequence - kNetworkState.ack_sequence >= MAX_NETQUEUE) continue;
    kNetworkState.ack_sequence = ack_sequence;
    kNetworkState.server_jerk = update->server_jerk;

    const uint8_t* offset = (kNetworkState.netbuffer + sizeof(NotifyUpdate));
    const uint8_t* end_buffer = kNetworkState.netbuffer + bytes_received;
    const uint64_t num_players = kNetworkState.num_players;
    while (offset + sizeof(NotifyFrame) < end_buffer) {
      NotifyFrame* nf = (NotifyFrame*)offset;
      offset += sizeof(NotifyFrame);
      const uint64_t frame = nf->frame;
      const uint64_t slot = NETQUEUE_SLOT(frame);

      if (ALAN) {
        printf(
            "CliRcvFrame "
            "[ %lu slot ] "
            "[ %lu frame ] "
            "\n",
            slot, frame);
      }

      for (int i = 0; i < num_players; ++i) {
        if (offset + sizeof(Turn) >= end_buffer) {
          kNetworkExit = kNeCorrupt;
          return;
        }
        Turn* turn = (Turn*)offset;
        const uint64_t event_bytes = turn->event_bytes;
        if (offset + event_bytes + sizeof(Turn) > end_buffer) {
          kNetworkExit = kNeCorrupt;
          return;
        }
        if (event_bytes > sizeof(PlatformEvent) * MAX_TICK_EVENTS) {
          kNetworkExit = kNeCorrupt;
          return;
        }
        if (kNetworkState.network_slot[slot][i] == kSlotInFlight) {
          const uint8_t* event = offset + sizeof(Turn);
          InputBuffer* ibuf = &kNetworkState.player_input[slot][i];
          memcpy(ibuf->input_event, event, event_bytes);
          ibuf->used_input_event = event_bytes / sizeof(PlatformEvent);
          static_assert(sizeof(PlatformEvent) % 2 == 0,
                        "Prefer a power of 2 for fast division.");
          kNetworkState.network_slot[slot][i] = kSlotReceived;
        }
        offset += event_bytes + sizeof(Turn);
      }
    }
  }

  uint64_t end_frame =
      next_simulation_frame + NetworkContiguousSlotReady(next_simulation_frame);
  kNetworkState.ack_frame = end_frame - 1;
}

uint64_t
NetworkQueueGoal()
{
  // No ack data yet, allow unbuffered play
  if (kNetworkState.egress_min == UINT64_MAX) return 1;

  // roundf may result in values less than rsdev_const
  // Add one because measure is done before current frame processing
  return roundf(StatsRsDev(&kNetworkStats) * kNetworkState.rsdev_const) + 1;
}

