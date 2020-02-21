#!/bin/bash
# Build headless with asan and symbols
DEV_FLAGS="-DHEADLESS -fsanitize=address -g -flto -Wl,-as-needed" ./cxx.sh src/space.cc

# Run
FRAMERATE=60
SECONDS=60
bin/space -f $((FRAMERATE*SECONDS)) -p 9800
echo "space exited with code $?"
