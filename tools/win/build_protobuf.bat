@echo off

cd third_party\protobuf
mkdir build_output
cd build_output
cmake -DCMAKE_BUILD_TYPE=Debug ..\ -G Ninja
ninja libprotobuf
cmake -DCMAKE_BUILD_TYPE=Release ..\ -G Ninja
ninja libprotobuf
ninja protoc

cd ..\..\..\
