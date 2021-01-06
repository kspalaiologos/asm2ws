#!/bin/sh

TEST_JIT=$#

_log() { echo -n "$(tput setaf 6)$2 $(tput setaf 5)$1$(tput sgr0) $(printf '.%.0s' $(seq 1 70))" | head -c 70; }
_ok() { echo "$(tput setaf 2) [OK] $(tput sgr0)"; }
_fail() { echo "$(tput setaf 1) [FAIL] $(tput sgr0)\n$1"; exit 1; }
_if_error() { [ $(wc -c < "$1.err") -ne 0 ] && _fail $(cat "$1.err"); rm -f "$1.err"; }

_interpreter() {
    _log "$1" "$3"
    ./wsi $2 "$1" \
        < $([ -f "$1.in" ] && echo -n "$1.in" || echo -n "/dev/null") \
        > "$1.aout" 2> "$1.err"
    _if_error "$1"
    [ ! -f "$1.out" ] && _fail "$1.out missing"
    delta=$(diff "$1.aout" "$1.out")
    [ $? -eq 1 ] && _fail "$delta"
    rm "$1.aout"; _ok
}

_build() {
    _log "$1" "[ASM]"
    ./wsi -m "$1" > "$1.ws" 2> "$1.err"
    [ $? -eq 1 ] && _fail $(cat "$1.err")
    rm -f "$1.err"; _ok
}

_disasm() {
    _log "$1" "[D/C]"
    ./wsi -d "$1" > "$1.asm" 2> "$1.err"
    [ $? -eq 1 ] && _fail $(cat "$1.err")
    rm -f "$1.err"; _ok
}

_run_jit() {
    [ $TEST_JIT -gt 0 ] && _interpreter "$1" "-j" "[JIT]"
    _interpreter "$1" "" "[RUN]"
}

_build_run() {
    _build "$1"
    _run_jit "$1.ws"
    rm -f "$1.ws"
}

_rebuild_run_single() {
    _run_jit "$1"
    _disasm "$1"
    _build_run "$1.asm"
    rm -f "$1.asm"
}

for f in tests/ws-run/*.ws; do _run_jit "$f"; done
for f in tests/ws-rebuild/*.ws; do _rebuild_run_single "$f"; done
for f in tests/ws-build-run/*.asm; do _build_run "$f"; done
