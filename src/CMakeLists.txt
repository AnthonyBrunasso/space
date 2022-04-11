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
  ../build/proto/entity.pb.cc
  ../build/proto/map.pb.cc
)

set_property(TARGET space PROPERTY CXX_STANDARD 17)

message("${CMAKE_BUILD_TYPE}")

if (UNIX)
  target_link_libraries(space X11 EGL GL)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(space -l:libprotobufd.a)
  else ()
    target_link_libraries(space -l:libprotobuf.a)
  endif()
endif (UNIX)

if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DUNICODE)
  add_definitions(-D_UNICODE)
  target_link_libraries(space user32 opengl32 gdi32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(space libprotobufd)
  else ()
    target_link_libraries(space libprotobuf)
  endif()
endif(MSVC)


