#!/bin/sh

# this is needed to know in which user home the .ssh folder needs to be mounted
adduser -D -u $(id -u) -g $(id -g) hostcontrold -s /bin/sh;
exec su - hostcontrold

/bin/hostcontrold
