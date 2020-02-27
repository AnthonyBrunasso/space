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

struct PlayerState {
  Udp4 peer;
  uint64_t num_players;
  uint64_t game_id;
  uint64_t last_active;
  uint64_t sequence;
  uint64_t player_id;
};
static PlayerState zero_player;

static bool running = true;
#define MAX_BUFFER (4 * 1024)
#define MAX_PLAYER 2
PlayerState player[MAX_PLAYER];
uint64_t next_game_id = 1;
bool game_ready;
Clock_t server_clock;
#define TIMEOUT_USEC (2 * 1000 * 1000)

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

void
drop_inactive_players(uint64_t rt_usec)
{
  for (int i = 0; i < MAX_PLAYER; ++i) {
    if (memcmp(&zero_player, &player[i], sizeof(PlayerState)) == 0) continue;
    if (rt_usec - player[i].last_active > TIMEOUT_USEC) {
      player[i] = PlayerState{};
      printf("Server dropped packet flow. [index %d]\n", i);
    }
  }
}

uint64_t
server_main(void* void_arg)
{
  ServerParam* arg = (ServerParam*)void_arg;

  // network thread affinity set to anything but core 0
  if (platform::thread_affinity_count() > 1) {
    platform::thread_affinity_clear(0);
    printf("Server thread may run on %d cores\n",
           platform::thread_affinity_count());
  }

  uint8_t in_buffer[MAX_BUFFER];
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
  uint64_t time_step = 1000;
  platform::clock_init(time_step, &server_clock);
  while (running) {
    uint16_t received_bytes;
    Udp4 peer;

    uint64_t sleep_usec;
    if (platform::clock_sync(&server_clock, &sleep_usec)) {
      realtime_usec += time_step;
    }
    if (!udp::ReceiveAny(location, MAX_BUFFER, in_buffer, &received_bytes,
                         &peer)) {
      if (udp_errno) running = false;
      if (udp_errno) printf("Server udp_errno %d\n", udp_errno);
      drop_inactive_players(realtime_usec);
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
      player[player_index].game_id = 0;
      player[player_index].last_active = realtime_usec;
      player[player_index].sequence = 0;

      int ready_players = 0;
      for (int i = 0; i < MAX_PLAYER; ++i) {
        if (player[i].game_id) continue;
        if (player[i].num_players != num_players) continue;
        ++ready_players;
      }

      if (ready_players >= num_players) {
        NotifyStart* response = (NotifyStart*)(in_buffer);

        uint64_t player_id = 0;
        for (int i = 0; i < MAX_PLAYER; ++i) {
          if (player[i].game_id) continue;
          if (player[i].num_players != num_players) continue;

          printf(
              "Server Greeting [index %d] [player_id %d] [player_count %d] "
              "[game_id %d]\n",
              i, player_id, num_players, next_game_id);
          response->player_id = player_id;
          response->player_count = num_players;
          response->game_id = next_game_id;
          udp::SendTo(location, player[i].peer, in_buffer, sizeof(NotifyStart));
          player[i].game_id = next_game_id;
          player[i].player_id = player_id;
          ++player_id;
        }
        ++next_game_id;
      }
    }

    // Filter Identified clients
    if (pidx == -1) continue;

    // Mark player connection active
    player[pidx].last_active = realtime_usec;

    // Filter for game-ready clients
    if (!player[pidx].game_id) continue;

    Turn* packet = (Turn*)in_buffer;
    uint64_t game_id = player[pidx].game_id;
#if 0
    printf(
        "SvrRcv [ %d socket ] [ %d bytes ] [ %lu sequence ] [ %lu game_id ]\n",
        location.socket, received_bytes, packet->sequence, game_id);
#endif
    // Require stream integrity
    if (packet->sequence - player[pidx].sequence != 1) continue;
    player[pidx].sequence = packet->sequence;

    // NotifyTurn
    uint64_t event_bytes = received_bytes - sizeof(Turn);
    uint8_t out_buffer[MAX_BUFFER];
    NotifyTurn* nt = (NotifyTurn*)out_buffer;
    nt->frame = packet->sequence;
    nt->player_id = player[pidx].player_id;
    nt->ack_sequence = packet->sequence;
    memcpy(nt->event, packet->event, event_bytes);

    // Echo bytes to game participants
    for (int i = 0; i < MAX_PLAYER; ++i) {
      if (player[i].game_id != game_id) continue;

      if (!udp::SendTo(location, player[i].peer, out_buffer,
                       event_bytes + sizeof(NotifyTurn))) {
        puts("server send failed");
        break;
      }
    }
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

