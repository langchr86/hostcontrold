#! /bin/sh

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

compiler=${1:-gcc}
version=${2:-10}
container_id=${3:-/tmp/hostcontrold-docker-id}

if [ -f "${container_id}" ]; then
  docker rm -f "$(cat ${container_id})"
fi

docker run \
  --detach \
  --privileged \
  --rm \
  --volume="${SCRIPT_DIR}/../":/home/hostcontrold:rw \
  hostcontrold:${compiler}-${version} > "${container_id}"
