#!/bin/sh

printf "$(tput setaf 5)test name:$(tput sgr0) "
read name

echo "program: "
cat > "tests/ws-build-run/$name.asm"

echo "input: "
cat > "tests/ws-build-run/$name.in"

echo "output: "
cat > "tests/ws-build-run/$name.out"
