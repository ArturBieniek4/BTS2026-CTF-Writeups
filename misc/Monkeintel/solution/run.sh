#!/bin/bash

binwalk --log results.json ../challenge/monke_thinkin.png
jq -r '
  .[]
  | select(.Analysis)
  | .Analysis.file_map[]
  | "dd if=../challenge/monke_thinkin.png of=\(.offset).\(.name) bs=1 skip=\(.offset) count=\(.size) status=progress"
' results.json | sh
zip2john *.zip > zip.hash
john zip.hash --wordlist=rockyou.txt
PASS="$(john --show zip.hash | awk -F: 'NR==1 {print $2}')"
unzip -P "$PASS" *.zip
python3 extract.py monke2n.png
