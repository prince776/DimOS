#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/dimos.kernel isodir/boot/dimos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "dimos" {
	multiboot /boot/dimos.kernel
}
EOF
grub-mkrescue -o dimos.iso isodir
