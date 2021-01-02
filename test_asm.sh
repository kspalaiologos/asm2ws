#!/bin/sh
data=$(cat /dev/stdin)
./wsi -m "./tests/$1" > "./tests/$1.ws"
./wsi -d "./tests/$1.ws" > "./tests/$1.ws.asm"
echo "$data" | ./wsi "./tests/$1.ws" | md5sum | grep "$2" > /dev/null
exit $?
