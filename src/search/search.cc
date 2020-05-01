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
    bool checked = false;
    uint32_t depth = 0;
  };
  // Visited Nodes
  PathMapNode path_map[kMapMaxWidth][kMapMaxHeight];
  // BFS queue.
  v2i queue[kMapMaxWidth * kMapMaxHeight];
  int queue_size;
};

static Search kSearch;

#include "bfs.cc"

}
