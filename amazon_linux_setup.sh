#!/bin/bash
if [ "$(id -u)" -ne 0 ]; then
  echo "Must be root to install dependencies"
  exit 1
fi

yum install libX11-devel
yum install mesa-libGL-devel
yum install mesa-libEGL-devel

ln -s `realpath bin`/space_server /usr/bin/space_server
cp space.service /etc/systemd/system/
systemctl enable space
