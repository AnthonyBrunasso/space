#include <cstdio>
#include <cstring>

#include "platform/platform.cc"
#include "protocol.cc"

static ThreadInfo thread;

struct ServerParam {
  const char* ip;
  const char* port;
};
static ServerParam thread_param;

#define MAX_GAMEQUEUE 128
#define GAMEQUEUE_SLOT(sequence) ((sequence) % MAX_GAMEQUEUE)
#define MAX_GAME 10
#define MAX_PLAYER 2
#define MAX_PACKET_IN 1024
#define MAX_PACKET_OUT (MAX_PLAYER * 1024)
#define TIMEOUT_USEC (2 * 1000 * 1000)

struct PlayerState {
  Udp4 peer;
  uint64_t num_players;
  uint64_t game_index = UINT64_MAX;
  uint64_t pending_game_id;
  uint64_t last_active;
  uint64_t sequence;
  uint64_t ack_frame;
  uint64_t player_id;
  uint64_t cookie;
  uint64_t cookie_mismatch;
  uint64_t latency_excess;
  uint64_t corrupted;
};
static PlayerState zero_player;
static PlayerState player[MAX_PLAYER];

struct Game {
  // Unique id of a game session or 0 when unused
  uint64_t game_id;
  // Players in game at launch
  uint64_t num_players;
  // Last simulation frame of the game session
  uint64_t last_frame;
  // Simulation frame confirmed by all participants
  uint64_t ack_frame;
  // Game data (no protocol wrapper)
  uint8_t slot[MAX_GAMEQUEUE][MAX_PLAYER][MAX_PACKET_IN];
  // Game byte count
  uint64_t used_slot[MAX_GAMEQUEUE][MAX_PLAYER];
};
static Game game[MAX_GAME];

static bool running = true;
static uint64_t next_game_id = time(0);

int
GetPlayerIndexFromPeer(Udp4* peer)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (memcmp(peer, &player[i].peer, sizeof(Udp4)) == 0) return i;
  }

  return -1;
}

int
GetNextPlayerIndex()
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (memcmp(&zero_player, &player[i], sizeof(PlayerState)) == 0) return i;
  }

  return -1;
}

int
GetGameIndex(uint64_t game_id)
{
  for (int i = 0; i < MAX_GAME; ++i) {
    if (game[i].game_id == game_id) return i;
  }
  for (int i = 0; i < MAX_GAME; ++i) {
    if (game[i].game_id == 0) return i;
  }

  return -1;
}

bool
PlayerInGame(uint64_t gidx)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (player[i].game_index == gidx) return true;
  }
  return false;
}

uint64_t
PlayerContiguousSequence(uint64_t pidx)
{
  uint64_t gidx = player[pidx].game_index;
  assert(gidx != UINT64_MAX);
  Game* g = &game[gidx];

  uint64_t slot = GAMEQUEUE_SLOT(g->last_frame + 1);
  uint64_t end_slot = GAMEQUEUE_SLOT(g->ack_frame);
  uint64_t count = 0;
  while (slot != end_slot) {
    if (!g->used_slot[slot][pidx]) {
      break;
    }
    slot = GAMEQUEUE_SLOT(slot + 1);
    count += 1;
  }

  return count;
}

void
prune_players(uint64_t rt_usec)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (memcmp(&player[i], &zero_player, sizeof(PlayerState)) == 0) continue;
    if (rt_usec - player[i].last_active > TIMEOUT_USEC) {
      printf(
          "Server dropped packet flow. [ index %d ] [ game_index %lu ] [ "
          "realtime_usec %lu ] [ last_active %lu ]\n",
          i, player[i].game_index, rt_usec, player[i].last_active);
      player[i] = {};
    }
    if (player[i].cookie_mismatch > 3) {
      printf("Server closed packet flow: cookie_mismatch. [index %d]\n", i);
      player[i] = {};
    }
    if (player[i].latency_excess > 3) {
      puts("Server closed packet flow: ack_frame latency gap is excessive");
      player[i] = {};
    }
    if (player[i].corrupted) {
      puts("Server closed packet flow: corrupted");
      player[i] = {};
    }
  }
}

void
prune_games()
{
  bool active_game[MAX_GAME] = {};

  for (int pidx = 0; pidx < MAX_PLAYER; ++pidx) {
    uint64_t game_index = player[pidx].game_index;
    if (game_index == UINT64_MAX) continue;
    active_game[game_index] = 1;
  }

  for (int gidx = 0; gidx < MAX_GAME; ++gidx) {
    if (active_game[gidx]) continue;
    uint64_t game_id = game[gidx].game_id;
    if (!game_id) continue;
    printf("Server removed game [ game_index %d ] [ game_id %lu ]\n", gidx,
           game_id);
    game[gidx] = {};
  }
}

bool
SendFrame(Udp4 location, uint64_t frame, uint64_t pidx, const Game* g)
{
  static uint8_t out_buffer[MAX_PACKET_OUT];
  uint64_t sidx = GAMEQUEUE_SLOT(frame);
  uint64_t num_players = player[pidx].num_players;
  uint64_t ack_sequence = player[pidx].sequence;

  NotifyFrame* nf = (NotifyFrame*)out_buffer;
  nf->frame = frame;
  nf->ack_sequence = ack_sequence;
  uint8_t* offset = out_buffer + sizeof(NotifyFrame);
  for (int j = 0; j < num_players; ++j) {
    NotifyTurn* nt = (NotifyTurn*)offset;
    uint8_t * event = offset+sizeof(NotifyTurn);
    uint64_t event_bytes = g->used_slot[sidx][j];
    memcpy(event, g->slot[sidx][j], event_bytes);
    nt->event_bytes = event_bytes;

    offset += sizeof(NotifyTurn) + event_bytes;
  }

  return (udp::SendTo(location, player[pidx].peer, out_buffer,
                      offset - out_buffer));
}

void
game_transmit(Udp4 location, uint64_t game_index)
{
  const Game* g = &game[game_index];
  const uint64_t game_id = g->game_id;
  if (!game_id) return;

  const uint64_t start_frame = g->ack_frame + 1;
  const uint64_t end_frame = g->last_frame;

  // Retransmission of NotifyTurn per player
  for (uint64_t send_frame = start_frame; send_frame <= end_frame;
       ++send_frame) {
    for (int pidx = 0; pidx < MAX_PLAYER; ++pidx) {
      if (player[pidx].game_index != game_index) continue;
      if (player[pidx].ack_frame >= send_frame) continue;

      if (!SendFrame(location, send_frame, pidx, g)) {
        printf("server send failed [ player_index %d ]\n", pidx);
      }
    }
  }
#if 1
  printf("Server transmit [ start_frame %lu ] [ end_frame %lu ]\n", start_frame,
         end_frame);
#endif
}

bool
game_update(uint64_t game_index)
{
  Game* g = &game[game_index];
  const uint64_t game_id = g->game_id;
  const uint64_t next_frame = g->last_frame + 1;
  if (!game_id) return false;

#if 0
  printf("Update game [ game_id %lu ] [ num_players %lu ]\n", g->game_id,
         g->num_players);
#endif
  uint64_t sidx = GAMEQUEUE_SLOT(next_frame);
  for (uint64_t i = 0; i < g->num_players; ++i) {
    if (g->used_slot[sidx][i] == 0) return false;
  }
  uint64_t new_ack_frame = UINT64_MAX;
  for (int pidx = 0; pidx < MAX_PLAYER; ++pidx) {
    if (player[pidx].game_index != game_index) continue;

    new_ack_frame = MIN(new_ack_frame, player[pidx].ack_frame);
  }

  // Clear acked slots for reuse
  for (uint64_t i = g->ack_frame; i < new_ack_frame; ++i) {
    uint64_t sidx = GAMEQUEUE_SLOT(i);
    for (int j = 0; j < g->num_players; ++j) {
      g->used_slot[sidx][j] = 0;
    }
  }

#if 1
  printf("Server game [ frame %lu ] [ ack_frame %lu ] [ new_ack_frame %lu ]\n",
         next_frame, g->ack_frame, new_ack_frame);
#endif

  g->last_frame = next_frame;
  g->ack_frame = new_ack_frame;

  return true;
}

uint64_t
server_main(void* void_arg)
{
  ServerParam* arg = (ServerParam*)void_arg;

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
  if (!udp::GetAddr4(arg->ip, arg->port, &location)) {
    puts("server: fail GetAddr4");
    puts(arg->ip);
    puts(arg->port);
    return 2;
  }

  printf("Server binding %s:%s\n", arg->ip, arg->port);
  if (!udp::Bind(location)) {
    puts("server: fail Bind");
    return 3;
  }

  uint64_t realtime_usec = 0;
  uint64_t time_step_usec = 8 * 1000;
  Clock_t server_clock;
  platform::clock_init(time_step_usec, &server_clock);
  while (running) {
    uint16_t received_bytes;
    Udp4 peer;

    uint64_t sleep_usec;
    if (platform::clock_sync(&server_clock, &sleep_usec)) {
      realtime_usec += time_step_usec;

      prune_players(realtime_usec);
      prune_games();

      bool ready[MAX_GAME] = {};
      for (int i = 0; i < MAX_GAME; ++i) {
        while (game_update(i)) {
          ready[i] = true;
        }
      }
      for (int i = 0; i < MAX_GAME; ++i) {
        if (!ready[i]) continue;
        game_transmit(location, i);
      }
      continue;
    }

    if (!udp::ReceiveAny(location, MAX_PACKET_IN, in_buffer, &received_bytes,
                         &peer)) {
      if (udp_errno) running = false;
      if (udp_errno) printf("Server udp_errno %d\n", udp_errno);
      platform::sleep_usec(sleep_usec);
      continue;
    }

    int pidx = GetPlayerIndexFromPeer(&peer);

    // Handshake packet
    if (received_bytes >= sizeof(Handshake) &&
        strncmp(GREETING, (char*)in_buffer, greeting_size) == 0) {
      // No room for clients on this server
      int player_index = GetNextPlayerIndex();
      if (player_index == -1) continue;
      // Duplicate handshake packet, idx already assigned
      if (pidx != -1) continue;

      Handshake* header = (Handshake*)(in_buffer);
      uint64_t num_players = header->num_players;
      printf("Server Accepted Handshake [index %d]\n", player_index);
      player[player_index].peer = peer;
      player[player_index].num_players = num_players;
      player[player_index].game_index = UINT64_MAX;
      player[player_index].pending_game_id = 0;
      player[player_index].last_active = realtime_usec;
      player[player_index].sequence = 0;

      int ready_players = 0;
      for (int i = 0; i < MAX_PLAYER; ++i) {
        if (player[i].pending_game_id) continue;
        if (player[i].num_players != num_players) continue;
        ++ready_players;
      }

      if (ready_players >= num_players) {
        NotifyGame* response = (NotifyGame*)(in_buffer);

        uint64_t player_id = 0;
        for (int i = 0; i < MAX_PLAYER; ++i) {
          if (player[i].pending_game_id) continue;
          if (player[i].num_players != num_players) continue;
          unsigned long long player_cookie;
          if (!RDRND(&player_cookie)) {
            puts("Server crypto rng failure");
            return 4;
          }

          printf(
              "Server Greeting [index %d] [player_id %d] [player_count %d] "
              "[next_game_id %d] [cookie 0x%llx]\n",
              i, player_id, num_players, next_game_id, player_cookie);
          response->player_id = player_id;
          response->player_count = num_players;
          response->game_id = next_game_id;
          response->cookie = player_cookie;
          udp::SendTo(location, player[i].peer, in_buffer, sizeof(NotifyGame));
          player[i].pending_game_id = next_game_id;
          player[i].player_id = player_id;
          player[i].cookie = player_cookie;
          ++player_id;
        }
        next_game_id += 1 + (next_game_id == 0);
      }
    }

    // Filter Identified clients
    if (pidx == -1) {
      continue;
    }

    // Mark player connection active
    player[pidx].last_active = realtime_usec;

    // Handle pending games
    uint64_t gidx = player[pidx].game_index;
    if (gidx == UINT64_MAX) {
      if (received_bytes == sizeof(BeginGame)) {
        BeginGame* bgPacket = (BeginGame*)(in_buffer);
        if (player[pidx].cookie != bgPacket->cookie) {
          player[pidx].cookie_mismatch += 1;
          puts("cookie mismatch");
          continue;
        }
        // two-way interest is agreed, copy pending_game_id to game_id
        uint64_t game_id = player[pidx].pending_game_id;
        gidx = GetGameIndex(game_id);
        if (gidx == -1) {
          puts("Server is out of space for Space");
          continue;
        }
        player[pidx].game_index = gidx;
        game[gidx].game_id = game_id;
        game[gidx].num_players = player[pidx].num_players;
        game[gidx].last_frame = 0;
        game[gidx].ack_frame = 0;
        printf("Server created Game [ game_index %lu ]\n", gidx);
        continue;
      }

#if 0
      puts("Unknown message during pending state");
#endif
      continue;
    }

    // Require address stability
    if (memcmp(&player[pidx].peer, &peer, sizeof(Udp4)) != 0) {
      puts("unhandled: player address changed");
      continue;
    }

    const Update* packet = (Update*)in_buffer;
#if 0
    printf(
        "SvrRcv Precheck "
        "[ %lu received_bytes ] "
        "[ %lu packet_sequence ] "
        "[ %lu packet_ack_frame ] "
        "\n",
        received_bytes, packet->sequence, packet->ack_frame);
#endif

    // Require stream integrity
    int64_t player_delta = packet->sequence - player[pidx].sequence;
    if (player_delta >= MAX_GAMEQUEUE) {
      // puts("packet sequence not relevant to player");
      continue;
    }

    // Verify relevance to game state
    int64_t game_delta = packet->sequence - game[gidx].last_frame;
    if (game_delta >= MAX_GAMEQUEUE) {
      // puts("packet seqeuence not relevant to game");
      continue;
    }

    // Packet OK - Check game synchronization
    uint64_t game_id = game[gidx].game_id;
    uint64_t pid = player[pidx].player_id;
    int64_t sync_delta = packet->sequence - game[gidx].ack_frame;
    if (sync_delta < 1) {
      printf(
          "Latency Excess [ %lu player_index ] [ %lu packet_sequence ] [ %lu "
          "ack_frame ] [ %lu clock_jerk ]\n",
          pidx, packet->sequence, game[gidx].ack_frame, server_clock.jerk);
      player[pidx].latency_excess += 1;
      continue;
    }

    // Handle storage of new packet in game
    player[pidx].ack_frame = MAX(player[pidx].ack_frame, packet->ack_frame);
    const uint8_t* read_offset = in_buffer + sizeof(Update);
    const uint8_t* end_buffer = in_buffer + received_bytes;
    uint64_t sequence = packet->sequence;
    uint64_t ack_sidx = GAMEQUEUE_SLOT(game[gidx].ack_frame);

#if 0
    printf(
        "Server processing "
        "[ %ld bytes ] "
        "[ %p read ] "
        "[ %p end ] "
        "\n",
        received_bytes, read_offset, end_buffer);
#endif

    while (read_offset < end_buffer) {
      uint64_t sidx = GAMEQUEUE_SLOT(sequence);

      // Prevent clobber of unprocessed turns
      if (sidx == ack_sidx) {
        break;
      }

      const Turn* turn = (const Turn*)read_offset;
      const uint8_t *event = read_offset+sizeof(Turn);
      uint64_t event_bytes = turn->event_bytes;

#if 0
      printf(
          "Server Apply "
          "[ sequence %lu ] "
          "[ event_bytes %lu ] "
          "\n",
          sequence, event_bytes);
#endif

      if (!game[gidx].used_slot[sidx][pid]) {
        // Apply turn data
        memcpy(game[gidx].slot[sidx][pid], event, event_bytes);
        game[gidx].used_slot[sidx][pid] = event_bytes;
      }

      // Advance
      read_offset += event_bytes + sizeof(Turn);
      sequence += 1;
    }

    if (read_offset == end_buffer) {
      // Player sequence can be determined after storage
      player[pidx].sequence =
          game[gidx].last_frame + PlayerContiguousSequence(pidx);
    } else {
      // Do not advance
      player[pidx].corrupted = 1;
    }

#if 0
    printf(
        "SvrRcv "
        "[ %d player_index ] "
        "[ %d bytes ] "
        "[ %lu packet_sequence ] "
        "[ %lu player_sequence ] "
        "[ %lu game_id ] "
        "\n",
        pidx, received_bytes, packet->sequence, player[pidx].sequence, game_id);
#endif
  }

  return 0;
}

bool
CreateNetworkServer(const char* ip, const char* port)
{
  if (thread.id) return false;

  thread.func = server_main;
  thread.arg = &thread_param;
  thread_param.ip = ip;
  thread_param.port = port;
  return platform::thread_create(&thread);
}

uint64_t
WaitForNetworkServer()
{
  if (!thread.id) return 0;

  platform::thread_join(&thread);
  return thread.return_value;
}

