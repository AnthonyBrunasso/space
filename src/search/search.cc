#pragma once

#include "math/vec.h"

namespace search {

constexpr uint32_t kMapMaxWidth = 64;
constexpr uint32_t kMapMaxHeight = 64;

struct Path {
  v2i queue[kMapMaxWidth * kMapMaxHeight];
  int size;
};

struct Search {
  struct PathMapNode {
    v2i from;
    uint32_t depth = 0;
    bool checked = false;
  };
  // Visited Nodes
  PathMapNode path_map[kMapMaxWidth][kMapMaxHeight];
  // BFS queue.
  v2i queue[kMapMaxWidth * kMapMaxHeight];
  int queue_size;
};

static Search kSearch;
static Path kPath;

#include "bfs.cc"

}
