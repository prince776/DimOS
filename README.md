# DimOS

A x86_64 Operating System. An OS with a future that's not quite bright.

## Features

- [x] Limine boot
- [x] Long Mode (64 bit), Higher Half Kernel
- [x] Software and Hardware Interrupts (IDT and PIT)
- [x] Physical Memory Manager (Bitmap based)
- [x] Virtual Memory Manager
- [x] Kernel Heap (FreeList)
- [x] malloc/free and new/delete support
- [x] ~~Minimal libc~~ printf
- [x] Kernel Threads with Context Switching
- [x] Preemptive Multitasking using timer interupts (PIT)
- [x] Round Robin Scheduler
- [x] Concurrency primitives (Mutex, Condition Variable, and Semaphore)
- [ ] File system
- [ ] Userland process and threads
- [ ] System Calls
- [ ] Shell
- [ ] Actual libc
- [ ] Network Stack
- [ ] Basic GUI...

Here's the kernel doing multithreaded addition on a shared variable with Mutex and Condition variables:

![Image](docs/demo.png "Demo Image")

## Build Instructions

### Requirements:

- Qemu for emulation (Ex: `brew install qemu`)
- x86_64_elf gcc cross compiler (Ex: `brew install x86_64-elf-gcc`)
- `xorriso` for iso creation (Ex: `brew install xorriso`)

### Build and run

- Clone the repo
- Run `git submodule update --init` (or clone recursively in first step itself)
- Run `make -C limine`
- Install `xorriso` (ex: `brew install xorriso`)
- Install `qemu` (ex: `brew install qemu`)
- Run `./all.sh` to clean + build + run.
- Use `./build.sh` for just building, `./qemu.sh` for just running
