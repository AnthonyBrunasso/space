#!/bin/bash
source ./run_constants.sh

if [ -n "$unix" ]; then
  echo Building with $CXX $CXXFLAGS
  time $CXX $CXXFLAGS $1 -I src/ -lX11 -lEGL -lGL -lpthread -o $BIN_DIR/`basename -s .cc $1`
else
  time $CXX $CXXFLAGS $1 -O0 -g -ObjC++ -I src/ -L bin/ -ldl -o $BIN_DIR/`basename -s .cc $1` -framework OpenGL -framework AppKit -mmacosx-version-min=10.7 -stdlib=libc++ -Wno-format
  # For teeny weeny builds '-Os -flto'
  # O0 takes about half a second off the compile time.
fi

# If a second arg exists run the binary
if [ ! -z "$2" ]; then
  ./$BIN_DIR/`basename -s .cc $1`
fi
