#!/bin/bash

TEST_JIT=0

# TODO: Capture stderr in wsi invocations.

if [[ $# -ne 0 ]]; then
    # one argument => test the JIT too.
    TEST_JIT=1
fi

_log() {
    echo -n "$(tput setaf 6)$2 $(tput setaf 5)$1$(tput sgr0) $(printf '.%.0s' {1..60})" | head -c 60
    echo -n 
}

_ok() {
    echo "$(tput setaf 2) [OK] $(tput sgr0)"
}

_fail() {
    echo "$(tput setaf 1) [FAIL] $(tput sgr0)"
    echo -n "$1"
    exit 1
}

_interpreter() {
    _log "$1" "$3"

    if [[ -f "$1.in" ]]; then
        ./wsi $2 "$1" < "$1.in" > "$1.aout"
    else
        ./wsi $2 "$1" > "$1.aout"
    fi

    delta=$(diff "$1.aout" "$1.out")
    status=$?

    if [[ $status -eq 1 ]]; then
        _fail "$delta"
    fi

    _ok

    rm "$1.aout"
}

_build() {
    _log "$1" "[ASM]"

    ./wsi -m "$1" > "$1.ws"
    status=$?

    if [[ $status -eq 1 ]]; then
        _fail ""
    fi

    _ok
}

_disasm() {
    _log "$1" "[D/C]"

    ./wsi -d "$1" > "$1.asm"
    status=$?

    if [[ $status -eq 1 ]]; then
        _fail ""
    fi

    _ok
}

_run_jit() {
    if [[ $TEST_JIT -eq 1 ]]; then
        _interpreter "$1" "-j" "[JIT]"
    fi

    _interpreter "$1" "" "[RUN]"
}

test_run_single () {
    _run_jit "$1"
}

test_build_run_single() {
    _build "$1"
    _run_jit "$1.ws"
    rm -f "$1.ws"
}

test_rebuild_run_single() {
    _run_jit "$1"
    _disasm "$1"
    _build "$1.asm"
    _run_jit "$1.asm.ws"
    rm -f "$1.asm" "$1.asm.ws"
}

for f in tests/ws-run/*.ws; do test_run_single "$f"; done
for f in tests/ws-rebuild/*.ws; do test_rebuild_run_single "$f"; done
for f in tests/ws-build-run/*.asm; do test_build_run_single "$f"; done
