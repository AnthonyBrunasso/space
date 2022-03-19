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
  ../build/proto/animation.pb.cc
)

if (UNIX)
  target_link_libraries(space X11 EGL GL)
endif (UNIX)

if (MSVC)
  set_property(TARGET space PROPERTY CXX_STANDARD 17)
  # Just match the runtime library of protobuf. Can investigate changing this if it becomes a problem.
  add_definitions(-DUNICODE)
  add_definitions(-D_UNICODE)
  target_link_libraries(space user32 opengl32 gdi32)
endif(MSVC)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_link_libraries(space libprotobufd)
else ()
  target_link_libraries(space libprotobuf)
endif()

