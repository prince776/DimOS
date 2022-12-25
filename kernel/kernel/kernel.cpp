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

// TODO: Yield is outdated, since KThread ContextStack was updated, use after fixing
extern "C" void yield(kernel::Thread*);

Vector<kernel::Thread>* kthreadsPtr = nullptr;
size_t currKThreadIdx = 0;

void kthread1() {
    // How to get the ptr to current thread's control block?
    printf("Thread type 1 executed\n");
    volatile int sum = 0;
    for (int i = 0; i < 30000000; i++) {
        sum |= i;
        if (i % 10000000 == 0) printf("Type 1 executed more\n");
    }
    // yield(&kernel::thisThread());
    printf("Thread type 1 executed final time\n");
    kernel::thisThread().finished = true;

    panic("Nothing to do for thread 1\n");
    // yield(&kernel::thisThread());
} // Must not return out of a kernel thread

void kthread2() {
    printf("Thread type 2 executed\n");
    volatile int sum = 0;
    for (int i = 0; i < 30000000; i++) {
        sum |= i;
        if (i % 10000000 == 0) printf("Type 2 executed more\n");
    }
    // yield(&kernel::thisThread());
    printf("Thread type 2 executed final time\n");
    kernel::thisThread().finished = true;
    // yield(&kernel::thisThread());
    panic("Nothing to do for thread 2\n");
} // Must not return out of a kernel thread

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

    Vector<kernel::Thread> local;
    kthreadsPtr = &local;

    // Insert a finished thread that represents curr thread.
    local.push_back(kernel::Thread(0));
    local[0].finished = true;

    local.push_back(kernel::Thread((uint64_t)&kthread1));
    local.push_back(kernel::Thread((uint64_t)&kthread2));


    for (auto& x : local) {
        printf("KThread: rsp: %x, rip: %x, id: %d \n", x.rsp, x.rip, x.id);
    }

    {
        registerInterruptHandler(pic::PIC1Offset, premptiveScheduler);
        pit::init(0xffff);
    }

    panic("Nothing to do\n");
}
