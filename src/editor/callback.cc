#pragma once

struct AssetSelection {
  Rectf tex_rect;
  Rectf world_rect;
  Rectf world_rect_scaled;
};

template <typename T>
using EditorCallbacks = std::vector<std::function<void(const T&)>>;

#define DEFINE_EDITOR_CALLBACK(name, param)                            \
  static EditorCallbacks<param> k##name;                               \
  void Subscribe##name(const std::function<void(const param&)> func) { \
    k##name.push_back(func);                                           \
  }                                                                    \
                                                                       \
  void Dispatch##name(const param& p) {                                \
     for (const auto& event : k##name) {                               \
      event(p);                                                        \
    }                                                                  \
  }
