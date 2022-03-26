#!/bin/bash

param_one="$1"

if [ ! -d "third_party/protobuf/build_output" ]; then
  source tools/unix/build_protobuf.sh
fi

if [ ! -d "build" ]; then
  mkdir build
fi

if [ ! -d "build/proto" ]; then
  source tools/unix/gen_proto.sh
fi

cd build

if [ ! -f "Makefile" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug ../
fi

make space -j 8

if [ $? -eq 0 ]; then
  ./src/space
fi
