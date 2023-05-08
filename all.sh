#!/bin/sh
set -e
./clean.sh
./build.sh
./qemu.sh


# x86_64-elf-gcc --sysroot=/Users/prince/projects/DimOS/sysroot -isystem=/usr/include -T arch/x86_64/linker.ld -o dimos.kernel -g -Wall -Wextra -ffreestanding -fstack-protector-strong -fno-lto -fno-pie -fno-pic -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mcmodel=kernel -MMD   -D__is_kernel -Iinclude -mgeneral-regs-only -fno-rtti -fno-exceptions   -static -z max-page-size=0x1000  arch/x86_64/boot.o arch/x86_64/gdt.o arch/x86_64/descriptor_tables.o arch/x86_64/kthread.o arch/x86_64/interrupt.o arch/x86_64/paging.o kernel/kernel.o kernel/common.o kernel/debug.o kernel/isr.o kernel/supcpp.o kernel/memory/pmm.o kernel/memory/vmm.o kernel/memory/heap.o kernel/memory/kheap.o kernel/cpp/memory.o kernel/cpp/common.o kernel/process/kthread.o kernel/filesystem/ramdisk.o kernel/filesystem/vfs.o kernel/concurrency/primitives.o kernel/devices/pic.o kernel/devices/pit.o kernel/devices/keyboard.o kernel/demo/demo.o  -nostdlib -lk -lgcc -nodefaultlibs 
