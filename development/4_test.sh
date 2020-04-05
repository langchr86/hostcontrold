#! /bin/sh -e

echo "build"
mkdir -p /tmp/build
cd /tmp/build
cmake /tmp/hostcontrold
make -j4

# TODO(clang)
# echo "unit tests"
# make test

echo "installation"
make install
systemctl daemon-reload

echo "first start fails but creates example config"
systemctl start hostcontrold | true
systemctl is-failed --quiet hostcontrold
ls /etc/hostcontrold.conf

echo "second start succeeds"
systemctl start hostcontrold
systemctl is-active --quiet hostcontrold
