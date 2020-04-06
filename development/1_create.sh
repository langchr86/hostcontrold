#! /bin/sh

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

compiler=${1:-gcc}
version=${2:-10}

docker build --rm \
  --file=${SCRIPT_DIR}/Dockerfile.${compiler} \
  --build-arg version=${version} \
  --tag=hostcontrold:${compiler}-${version} \
  .
