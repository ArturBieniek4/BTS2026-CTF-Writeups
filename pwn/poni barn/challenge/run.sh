#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

exec qemu-system-aarch64 \
    -M virt \
    -cpu cortex-a53 \
    -m 128M \
    -nographic \
    -nic none \
    -kernel rom.elf
