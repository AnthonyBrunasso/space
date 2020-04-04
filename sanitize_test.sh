#!/bin/bash
# Build headless with asan and symbols
DEV_FLAGS="-DHEADLESS -fsanitize=address -g -flto -Wl,-as-needed" ./cxx.sh src/space.cc

# Run
FRAMERATE=60
SECONDS=60
bin/space -l $((FRAMERATE*SECONDS)) -i 127.0.0.1
echo "space exited with code $?"
