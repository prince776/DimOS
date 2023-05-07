#!/bin/sh
set -e
# . ./build.sh

###### OLD GRUB #####
# mkdir -p isodir
# mkdir -p isodir/boot
# mkdir -p isodir/boot/grub

# cp sysroot/boot/dimos.kernel isodir/boot/dimos.kernel
# cat > isodir/boot/grub/grub.cfg << EOF
# menuentry "dimos" {
# 	multiboot /boot/dimos.kernel
# }
# EOF
# grub-mkrescue -o dimos.iso isodir

# Download the latest Limine binary release.
# git clone https://github.com/limine-bootloader/limine.git --branch=v4.x-branch-binary --depth=1
make -C limine
mkdir -p isodir

cp -v sysroot/boot/dimos.kernel limine.cfg limine/limine.sys \
      limine/limine-cd.bin limine/limine-cd-efi.bin isodir/
 
# Create the bootable ISO.
xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-cd-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        isodir -o dimos.iso
 
# Install Limine stage 1 and 2 for legacy BIOS boot.
./limine/limine-deploy dimos.iso
