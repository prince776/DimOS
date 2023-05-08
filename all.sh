#!/bin/sh
set -e
./clean.sh
./build.sh
./qemu.sh
