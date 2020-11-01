file(GLOB proto_files "*.proto")

# Generates all the compilable proto files "*.pb.h, *.pb.cc" before protoc runs
# so the file list can be added as dependencies to the service executable.
foreach (proto ${proto_files})
  get_filename_component(proto_name ${proto} NAME_WE)
  list(APPEND proto_src ${PROJECT_BINARY_DIR}/generated/${proto_name}.pb.h ${PROJECT_BINARY_DIR}/generated/${proto_name}.pb.cc)
  list(APPEND grpc_src ${PROJECT_BINARY_DIR}/generated/${proto_name}.grpc.pb.h ${PROJECT_BINARY_DIR}/generated/${proto_name}.grpc.pb.cc)
endforeach()

# Run protoc for each proto input.
foreach (proto ${proto_files})
  get_filename_component(proto_name ${proto} NAME)
  get_filename_component(proto_name_we ${proto} NAME_WE)
  set(output_cc ${PROJECT_BINARY_DIR}/generated/${proto_name_we}.pb.cc)
  set(output_h ${PROJECT_BINARY_DIR}/generated/${proto_name_we}.pb.h)

  # Generate protobuf / grpc outputs.
  add_custom_command(
    DEPENDS protoc
    COMMAND ${PROJECT_BINARY_DIR}/bin/protoc -I=${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${PROJECT_BINARY_DIR}/generated ${proto}
    OUTPUT ${output_cc} ${output_h}
  )

  # If a proto file was added that had no grpc markup this command
  # would need to be modified to only run for those that have it.
  set(output_grpc_cc ${PROJECT_BINARY_DIR}/generated/${proto_name_we}.grpc.pb.cc)
  set(output_grpc_h ${PROJECT_BINARY_DIR}/generated/${proto_name_we}.grpc.pb.h)

  # Because msvc generates executables with a '.exe' extension.
  if (MSVC)
    set(grpc_plugin ${PROJECT_BINARY_DIR}/bin/grpc_cpp_plugin.exe)
  else()
    set(grpc_plugin ${PROJECT_BINARY_DIR}/bin/grpc_cpp_plugin)
  endif()

  # Generate protobuf / grpc outputs.
  add_custom_command(
    DEPENDS protoc
    COMMAND ${PROJECT_BINARY_DIR}/bin/protoc -I=${CMAKE_CURRENT_SOURCE_DIR} --plugin=protoc-gen-grpc=${grpc_plugin} --grpc_out=${PROJECT_BINARY_DIR}/generated ${proto}
    OUTPUT ${output_grpc_cc} ${output_grpc_h}
  )
endforeach()

include_directories(SYSTEM "${PROJECT_BINARY_DIR}/generated")
include_directories(SYSTEM "./")

# Setup MSVC parallelized builds
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_executable(4xserver ${proto_src} ${grpc_src} 4xserver.cc)
# image_service need not link protoc / grpc_cpp_plugin but it does require them
# built in order to successfully complete code generation.
add_dependencies(4xserver protoc grpc_cpp_plugin)
# absl::strings and absl_flags to support specifying FLAGS in our binary
# with ABSL_FLAG.
target_link_libraries(4xserver grpc++ absl::strings absl_flags absl_flags_parse)

add_executable(4xclient ${proto_src} ${grpc_src} 4xclient.cc)
add_dependencies(4xclient protoc grpc_cpp_plugin)
target_link_libraries(4xclient grpc++ absl::strings absl_flags absl_flags_parse)

add_executable(4xcli ${proto_src} ${grpc_src} 4xcli.cc)
add_dependencies(4xcli protoc grpc_cpp_plugin)
target_link_libraries(4xcli grpc++ absl::strings absl_flags absl_flags_parse)
