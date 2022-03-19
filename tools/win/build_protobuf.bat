@echo off

cd third_party\protobuf
if not exist build_output mkdir build_output
cd build_output
cmake -DCMAKE_BUILD_TYPE=Debug -Dprotobuf_MSVC_STATIC_RUNTIME=OFF ..\ -G Ninja
ninja libprotobuf
cmake -DCMAKE_BUILD_TYPE=Release -Dprotobuf_MSVC_STATIC_RUNTIME=OFF ..\ -G Ninja
ninja libprotobuf
ninja protoc

cd ..\..\..\
