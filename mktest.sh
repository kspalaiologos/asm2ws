#!/bin/bash

_prompt() {
    echo "$(tput setaf 7)$1:$(tput sgr0) "
    cat > "$name.$2"
}

printf "$(tput setaf 7)test name:$(tput sgr0) "
read name

read -n 1 -p "$(tput setaf 7)type: (r)ebuild/(b)uild$(tput sgr0) " ans;

echo ""

cd tests

case $ans in
    r|R)
        cd "ws-rebuild"

        _prompt "program" "bak"
        _prompt "input" "ws.in"
        _prompt "output" "ws.out"

        cp "$name.ws.in" "$name.ws.asm.ws.in"
        cp "$name.ws.out" "$name.ws.asm.ws.out"

        [ $(wc -c < "$name.ws.in") -eq 0 ] \
            && rm "$name.ws.in" \
            && "$name.ws.asm.ws.in"

        ../../wsi -m "$name.bak" > "$name.ws"
        ;;
    b|B)
        cd "ws-build-run"

        _prompt "program" "asm"
        _prompt "input" "ws.in"
        _prompt "output" "ws.out"

        [ $(wc -c < "$name.in") -eq 0 ] && rm "$name.in"
        ;;
    *)
        exit;;
esac
