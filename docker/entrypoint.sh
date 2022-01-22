#!/bin/bash -ex

mount

# this is needed to know in which user home the .ssh folder needs to be mounted
adduser -D -u ${UID} hostcontrold -s "/bin/bash" | true
su - hostcontrold

/bin/hostcontrold
