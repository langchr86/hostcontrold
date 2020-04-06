#! /bin/sh

container_id=${1:-/tmp/hostcontrold-docker-id}

docker rm -f "$(cat ${container_id})"
rm -f ${container_id}

docker rmi -f $(docker images --filter=reference="hostcontrold:*" -q)

docker image prune --force
