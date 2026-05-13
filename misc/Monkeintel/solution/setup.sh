#!/bin/bash
set -euo pipefail

sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  john binwalk python3-pil
wget https://github.com/brannondorsey/naive-hashcat/releases/download/data/rockyou.txt
