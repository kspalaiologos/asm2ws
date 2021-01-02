#!/bin/sh
./wsi $1 ./tests/$2 > /dev/null || exit 0
exit 1
