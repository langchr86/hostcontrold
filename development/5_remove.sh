#! /bin/sh

container_id=${1:-/tmp/hostcontrold-docker-id}

docker rm -f "$(cat ${container_id})"
rm -f ${container_id}

docker image prune --force
