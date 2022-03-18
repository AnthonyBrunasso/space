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

if (MSVC)
  set_property(TARGET space PROPERTY CXX_STANDARD 17)
  add_definitions(-DUNICODE)
  add_definitions(-D_UNICODE)
  target_link_libraries(space user32 opengl32 gdi32 libprotobuf)
endif(MSVC)
