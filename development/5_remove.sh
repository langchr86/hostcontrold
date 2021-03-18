#! /bin/bash -e

docker rm -f test-container

docker rmi -f $(docker images --filter=reference="hostcontrold:*" -q)

docker image prune --force
