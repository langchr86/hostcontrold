#! /bin/sh

container_id=${1:-/tmp/hostcontrold-docker-id}

docker exec -it "$(cat ${container_id})" /bin/bash
