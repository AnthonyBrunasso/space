#!/bin/bash

if [ ! -d "build/proto" ]; then
  mkdir build/proto
fi

for f in proto/*.proto ; do
  third_party/protobuf/build_output/protoc -I=proto/ --cpp_out=build/proto/ $f
done
