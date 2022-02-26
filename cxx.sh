#!/bin/bash
source ./bash_constants.sh

set -o xtrace

if [ $# -eq 0 ]; then
  echo 'Usage: $0 <cc_file>'
  exit 1
fi

FILEEXT=.${1##*.}
APP=$BIN_DIR/`basename -s $FILEEXT $1`

if [ -n "$unix" ]; then
  echo Building with $CXX $CXXFLAGS
  time $CXX -g1 $CXXFLAGS $1 third_party/imgui/imgui.cpp third_party/imgui/imgui_draw.cpp third_party/imgui/imgui_tables.cpp third_party/imgui/imgui_widgets.cpp third_party/imgui/imgui_demo.cpp -I src/ -I third_party/imgui/ -lX11 -lEGL -lGL -lpthread -o $APP
else
  time $CXX -O2 $CXXFLAGS $1 -I . -I src/ -ObjC++ -L bin/ -ldl -o $APP -framework OpenGL -framework AppKit -framework OpenAL -mmacosx-version-min=10.14 -stdlib=libc++ -Wno-format -Wno-deprecated-declarations
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
