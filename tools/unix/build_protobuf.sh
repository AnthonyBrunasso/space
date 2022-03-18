#!/bin/bash

cd third_party/protobuf
if [ ! -d "build_output" ]; then
  mkdir build_output
fi
cd build_output
cmake -DCMAKE_BUILD_TYPE=Debug ../
make libprotobuf -j 8
cmake -DCMAKE_BUILD_TYPE=Release ../
make libprotobuf -j 8
make protoc -j 8

cd ../../../
