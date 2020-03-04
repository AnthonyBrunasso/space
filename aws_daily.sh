#!/bin/bash
pushd `dirname $0`
echo space_server update: `date +%H:%M:%S`

# DEV_FLAGS Remove X11/EGL dependencies at link time
DEV_FLAGS="-flto -Wl,-as-needed" ./cxx.sh src/space_server.cc

sudo systemctl restart space

popd
