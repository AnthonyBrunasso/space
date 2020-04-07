#!/bin/bash
source ./bash_constants.sh

if [ $# -eq 0 ]; then
  echo 'Usage: $0 <cc_file>'
  exit 1
fi

FILEEXT=.${1##*.}
APP=$BIN_DIR/`basename -s $FILEEXT $1`

if [ -n "$unix" ]; then
  echo Building with $CXX $CXXFLAGS
  time $CXX $CXXFLAGS $1 -I src/ -lX11 -lEGL -lGL -lpthread -o $APP
else
  time $CXX $CXXFLAGS $1 -I src/ -O1 -ObjC++ -L bin/ -ldl -o $APP -framework OpenGL -framework AppKit -mmacosx-version-min=10.7 -stdlib=libc++ -Wno-format
  # For teeny weeny builds '-Os -flto'
  # O0 takes about half a second off the compile time.
fi

# If more than one argument, and build succeeded
if [ $? -eq 0 -a $# -gt 1 ]; then
  # pop the first argument
  shift
  # pass remaining arguments to the app
  ./$APP $@
fi
