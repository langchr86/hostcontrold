#! /bin/sh

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

distribution=${1:-ubuntu}
version=${2:-bionic}
container_id=${3:-/tmp/hostcontrold-docker-id}

docker run \
  --detach \
  --privileged \
  -v /sys/fs/cgroup:/sys/fs/cgroup:ro \
  --volume="${SCRIPT_DIR}/../":/tmp/hostcontrold:rw \
  ${distribution}-${version}:hostcontrold > "${container_id}"
