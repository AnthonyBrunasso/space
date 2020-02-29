#!/bin/bash
pushd `dirname $0`
echo space_server update: `date +%H:%M:%S`
docker-compose down
./docker_build.sh
docker-compose up -d
./sanitize_test.sh
popd
