#!/bin/bash -e

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

# see: https://docs.docker.com/desktop/multi-arch/

# first create new builder:
# docker buildx create --name multi-arch-builder --bootstrap

# setup QEMU:
# docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

# login to docker hub:
# docker login --username langchr86 --password-stdin
# copy-paste password and finish with ctrl-d ctrl-d

# later remove the credentials
# rm -rf /home/vagrant/.docker/config.json

cd ${SCRIPT_DIR}/..
docker buildx build \
  --builder multi-arch-builder \
  --platform linux/amd64,linux/arm64,linux/arm/v7 \
  --push \
  --tag langchr86/hostcontrold:latest \
  --tag langchr86/hostcontrold:1.0.0 \
  --file docker/Dockerfile \
  .
