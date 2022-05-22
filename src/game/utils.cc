#pragma once

template <typename Key, typename Value>
Value* FindOrNull(std::unordered_map<Key, std::unique_ptr<Value>>& map, const Key& key) {
  auto v = map.find(key);
  if (v == map.end()) return nullptr;
  return v->second.get();
}
