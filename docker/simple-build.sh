#!/bin/bash -e

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

cd ${SCRIPT_DIR}/..
docker build -t langchr86/hostcontrold:latest -f docker/Dockerfile .
