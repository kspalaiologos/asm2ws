#!/bin/sh
./wsi "$1" "./tests/$2" | md5sum | grep "$3" > /dev/null
exit $?
