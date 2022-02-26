#!/bin/bash

if [ ! -d "build" ]; then
  mkdir build
fi

cd build

if [ ! -f "Makefile" ]; then
  cmake ../
fi

make -j 8

if [ $? -eq 0 ]; then
  ./src/space
fi
