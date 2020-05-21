#! /bin/bash -e

echo "build"
mkdir -p /home/build
cd /home/build
cmake /home/hostcontrold
cmake --build /home/build --parallel 4

echo "unit tests"
cmake --build /home/build -- test

echo "installation"
cmake --build /home/build -- install
systemctl daemon-reload

echo "first start fails but creates example config"
systemctl start hostcontrold | true
sleep 1
systemctl is-failed --quiet hostcontrold
ls /etc/hostcontrold.conf

echo "second start succeeds"
systemctl start hostcontrold
sleep 1
systemctl is-active --quiet hostcontrold
