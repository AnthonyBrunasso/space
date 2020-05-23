#pragma once

#include "math/vec.h"

namespace search {

constexpr u32 kMapMaxWidth = 64;
constexpr u32 kMapMaxHeight = 64;

struct Path {
  v2i queue[kMapMaxWidth * kMapMaxHeight];
  s32 size;
};

struct Search {
  struct PathMapNode {
    v2i from;
    u32 depth = 0;
    b8 checked = false;
  };
  // Visited Nodes
  PathMapNode path_map[kMapMaxWidth][kMapMaxHeight];
  // BFS queue.
  v2i queue[kMapMaxWidth * kMapMaxHeight];
  s32 queue_size;
};

static Search kSearch;
static Path kPath;

#include "bfs.cc"

}
