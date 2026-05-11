#!/bin/bash

EXPLOIT_FILE=$(pwd)/exploit.in

pushd ../challenge >/dev/null

cat "$EXPLOIT_FILE" | timeout 5s ./run.sh | awk -F'|' '/^VAL:/ { printf "%s", $2; fflush() }'

popd
