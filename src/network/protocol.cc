#pragma once

#include <cstdint>

#include "platform/platform.cc"

const uint64_t greeting_size = 8;
#define GREETING "spacehi"
#define BEGINGAME "spacegame"

struct Handshake {
  const char greeting[greeting_size] = {GREETING};
  uint64_t num_players;
};

struct NotifyGame {
  uint64_t game_id;
  uint64_t player_id;
  uint64_t player_count;
  uint64_t cookie;
};

struct BeginGame {
  uint64_t cookie;
  uint64_t game_id;
};

struct Turn {
  uint64_t event_bytes;
#ifndef _WIN32
  PlatformEvent event[];
#endif
};

struct Update {
  uint64_t sequence;
  uint64_t ack_frame;
#ifndef _WIN32
  // N updates from the local player
  Turn turn[];
#endif
};

struct NotifyFrame {
  uint64_t frame;
#ifndef _WIN32
  // N updates, one for each game participant on frame
  Turn turn[];
#endif
};

struct NotifyUpdate {
  uint64_t ack_sequence;
#ifndef _WIN32
  NotifyFrame turn[];
#endif
};
