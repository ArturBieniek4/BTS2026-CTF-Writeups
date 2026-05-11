#!/bin/bash
exec socat \
    TCP-LISTEN:1337,reuseaddr,fork \
    EXEC:"timeout 120 qemu-system-aarch64 -M virt -cpu cortex-a53 -m 128M -nographic -monitor none -nic none -kernel /home/ctf/rom.elf",pty,rawer,stderr
