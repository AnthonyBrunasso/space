#!/bin/bash
# Removes X11/EGL dependencies at link time
DEV_FLAGS="-flto -Wl,-as-needed" ./cxx.sh src/space_server.cc
docker build . -t space
