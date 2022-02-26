include_directories(SYSTEM "./")
# Setup MSVC parallelized builds
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)
add_executable(space
  space.cc
  ../third_party/imgui/imgui.cpp
  ../third_party/imgui/imgui.cpp
  ../third_party/imgui/imgui_draw.cpp
  ../third_party/imgui/imgui_tables.cpp
  ../third_party/imgui/imgui_widgets.cpp
  ../third_party/imgui/imgui_demo.cpp
)

if (UNIX)
  target_link_libraries(space X11 EGL GL)
endif (UNIX)
