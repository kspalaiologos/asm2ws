#!/bin/sh
data=`cat /dev/stdin`
./wsi -m ./tests/$1 > ./tests/$1.ws
echo "$data" | ./wsi ./tests/$1.ws | md5sum | grep "$2" > /dev/null
exit $?
