// namespace live {

typedef std::function<std::vector<v2i>(v2i)> NeighborsFunc;
typedef std::function<bool(v2i)> ExpandFunc;

struct BfsSearchItr {
  BfsSearchItr(v2i start, const ExpandFunc& expand_func, const NeighborsFunc& neighbors_func)
      : expand_func(expand_func), neighbors_func(neighbors_func)
  {
    if (expand_func(start)) {
      nodes.reserve(16);
      to_test.reserve(32);
      to_test.push_back(start);
    }
    explored.insert(start);
  }

  bool
  Next()
  {
    if (to_test.empty()) return false;
    std::vector<v2i> new_to_test;
    for (v2i node : to_test) {
      nodes.push_back(node);
      std::vector<v2i> neighbors = neighbors_func(node);
      for (v2i neighbor : neighbors) {
        if (explored.find(neighbor) != explored.end()) continue;
        if (expand_func(neighbor)) {
          new_to_test.push_back(neighbor);
          explored.insert(neighbor);
        }
      }
    }
    to_test = std::move(new_to_test);
    return !to_test.empty();
  }

  std::vector<v2i> nodes;
  std::vector<v2i> to_test;
  std::unordered_set<v2i> explored;
  ExpandFunc expand_func;
  NeighborsFunc neighbors_func;
};

// }
