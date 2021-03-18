#! /bin/bash -e

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

compiler=${1:-gcc}
version=${2:-10}

docker run \
  --detach \
  --privileged \
  --rm \
  --volume="${SCRIPT_DIR}/../":/home/hostcontrold:rw \
  --name=test-container \
  hostcontrold:${compiler}-${version}
