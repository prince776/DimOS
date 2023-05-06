#include <stdint-gcc.h>
#include <kernel/common.h>
#include <stdlib.h>
#include <stddef.h>
#include <kernel/limine.h>
#include <stdio.h>
#include <kernel/debug.h>
#include <demo/demo.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/kheap.h>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/vector.hpp>
#include <kernel/process/kthread.h>
#include <kernel/devices/pit.h>
#include <kernel/isr.h>
#include <kernel/concurrency/primitives.h>
#include <kernel/filesystem/vfs.h>
#include <demo/terminal.hpp>
#include <kernel/devices/keyboard.h>

extern "C" void (*__init_array_start)(), (*__init_array_end)();

volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static volatile limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile limine_kernel_address_request kerenel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};
char* limineMemTypeMap[] = {
"USABLE",
"RESERVED",
"ACPI_RECLAIMABLE",
"ACPI_NVS",
"BAD_MEMORY",
"BOOTLOADER_RECLAIMABLE",
"KERNEL_AND_MODULES",
"FRAMEBUFFER",
};

uint64_t HHDMOffset;

extern "C" void kernel_early() {
    for (auto ctor = &__init_array_start; ctor < &__init_array_end; ctor++) {
        (*ctor)();
    }
    // Ensure we got a terminal
    if (terminal_request.response == NULL
        || terminal_request.response->terminal_count < 1) {
        panic("No terminal found");
    }
}

Vector<kernel::Thread> kthreads;
size_t currKThreadIdx = 0;

vfs::VFS globalVFS;

extern "C" void kernel_main(void) {

    // *************** Parse the Memory Map **************************//
    auto memArr = memmap_request.response->entries;
    auto memArrLen = memmap_request.response->entry_count;
    PhysicalMemMap physcialMemMap;
    physcialMemMap.totalSize = memArr[memArrLen - 2]->base + memArr[memArrLen - 2]->length;
    printf("Physcial Memory Map: (Total: %x) ------------------------------------\n", physcialMemMap.totalSize);
    for (int i = 0; i < memArrLen; i++) {
        auto memEntry = memArr[i];
        printf("Memory entry %d: base: %x length: %x type: %s\n", i, memEntry->base, memEntry->length, limineMemTypeMap[memEntry->type]);
        if (memEntry->type == LIMINE_MEMMAP_USABLE) {
            physcialMemMap.availMemArrCnt++;
        }
    }
    MemRange physicalRange[physcialMemMap.availMemArrCnt];
    for (int i = 0, j = 0; i < memArrLen; i++) {
        auto memEntry = memArr[i];
        if (memEntry->type == LIMINE_MEMMAP_USABLE) {
            physicalRange[j].start = memEntry->base;
            physicalRange[j].size = memEntry->length;
            j++;
        }
        else if (memEntry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) {
            physcialMemMap.kernelStart = memEntry->base;
            physcialMemMap.kernelSize = memEntry->length;
        }
    }
    physcialMemMap.availableMemArr = &physicalRange[0];
    for (int i = 0; i < physcialMemMap.availMemArrCnt; i++) {
        auto& memEntry = physcialMemMap.availableMemArr[i];
    }

    auto kernelAddr = kerenel_address_request.response;
    auto hhdmRes = hhdm_request.response;
    HHDMOffset = hhdmRes->offset;
    printf("Kernel Addr-> Physical: %x Virtual: %x\n", kernelAddr->physical_base, kernelAddr->virtual_base);
    printf("HHDM offset: %x\n", hhdmRes->offset);
    printf("------------------------------------------------------------------------\n");
    // ******************************************************************************//

    auto pmmMemRange = PMM::get().init(physcialMemMap);
    VMM::get().init(physcialMemMap, &pmmMemRange, 1);
    MallocHeap::init();
    printf("Memory Management System Activated!\n");

    // Intialize file system.
    auto rdfs = demo::Terminal::createRamdisk(50, 50);
    globalVFS = vfs::VFS((FileSystem*)&rdfs);

    // Initialize kernel thread related stuff in fs
    globalVFS.mkdir("/proc");

    // Initialize devices
    Keyboard::get().install();

    // Insert a finished thread that represents curr thread.
    kthreads.push_back(kernel::Thread(0));
    kthreads[0].state = TaskState::COMPLETED;


    auto terminal = demo::Terminal("prince");

    // terminal.cd("proc");
    // terminal.ls();

    // auto& fd = kernel::thisThread().fileDescriptors[0];
    // int buffer = 997;
    // fd.write(sizeof(buffer), (uint8_t*)&buffer);
    // long long newBuf = 0;
    // fd.read(sizeof(buffer), (uint8_t*)&newBuf);

    // printf("Read data: %d\n", newBuf);
    terminal.run();


    {
        // registerInterruptHandler(pic::PIC1Offset, premptiveScheduler);
        // pit::init(1);
    }
    panic("Nothing to do\n");
}

