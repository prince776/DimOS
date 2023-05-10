#!/bin/sh
set -e
. ./iso.sh

qemu-system-x86_64 -m 2G -cdrom dimos.iso -s -S
# -monitor stdio
# -s -S
