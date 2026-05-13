#!/bin/bash
set -euo pipefail

sudo apt-get update
sudo apt-get install -y \
  qemu-system-arm

qemu-system-aarch64 --version
