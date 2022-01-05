#! /bin/bash

DEPLOY_BUILD="/tmp/googletest"

clear_build_artefacts() {
  echo "remove all build artefacts in: ${DEPLOY_BUILD}"
  rm -rf ${DEPLOY_BUILD}
}
trap clear_build_artefacts EXIT

# do not deploy shared objects because linking to gtest.so and gmock.so will lead to ODR violation
cd /tmp \
  && git clone https://github.com/google/googletest.git \
  && cd googletest \
  && git checkout release-1.11.0 \
  && mkdir build \
  && cd build \
  && cmake -DBUILD_SHARED_LIBS=OFF .. \
  && make -j4 install \
  && ldconfig
