#!/bin/bash

param_one="$1"

if [ ! -d "build" ]; then
  mkdir build
fi

cd build

if [ ! -f "Makefile" ]; then
  cmake ../
fi

make space -j 8

if [ $? -eq 0 ]; then
  ./src/space
fi
