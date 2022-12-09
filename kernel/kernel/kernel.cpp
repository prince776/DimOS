#include <stdint-gcc.h>
#include <kernel/common.h>
#include <stdlib.h>
#include <stddef.h>
#include <kernel/limine.h>
#include <stdio.h>
#include <kernel/debug.h>
#include <demo/demo.h>
#include <kernel/memory/pmm.h>

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

extern "C" void kernel_main(void) {

    // *************** Parse the Memory Map **************************//
    auto memArr = memmap_request.response->entries;
    auto memArrLen = memmap_request.response->entry_count;
    PhysicalMemMap physcialMemMap;
    for (int i = 0; i < memArrLen; i++) {
        auto memEntry = memArr[i];
        if (memEntry->type == LIMINE_MEMMAP_USABLE) {
            physcialMemMap.totalSize = memEntry->base + memEntry->length;
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
    }
    physcialMemMap.availableMemArr = &physicalRange[0];
    printf("Physcial Memory Map: Total(%u) ------------------------------------\n", physcialMemMap.totalSize);
    for (int i = 0; i < physcialMemMap.availMemArrCnt; i++) {
        auto& memEntry = physcialMemMap.availableMemArr[i];
        printf("Memory entry %d: base: %x length: %x\n", i, memEntry.start, memEntry.size);
    }
    printf("------------------------------------\n");

    auto kernelAddr = kerenel_address_request.response;
    auto hhdmRes = hhdm_request.response;
    HHDMOffset = hhdmRes->offset;
    // ******************************************************************************//
    printf("Kernel Addr-> Physical: %x Virtual: %x\n", kernelAddr->physical_base, kernelAddr->virtual_base);
    printf("HHDM offset: %x\n", hhdmRes->offset);

    __asm__ volatile ("int $0x10");

    // PMM::get().init(physcialMemMap);

    // PhysicalAddr testFrame = PMM::get().allocFrame();
    // printf("Allocated memory: %x, %x\n", testFrame, testFrame + HHDMOffset);
    // testFrame = PMM::get().allocFrame();
    // printf("Allocated memory: %x, %x\n", testFrame, testFrame + HHDMOffset);
    // testFrame = PMM::get().allocFrame();
    // printf("Allocated memory: %x, %x\n", testFrame, testFrame + HHDMOffset);
    // testFrame = PMM::get().allocFrame();
    // printf("Allocated memory: %x, %x\n", testFrame, testFrame + HHDMOffset);
    // VMM::get().init();
    // void* newPage = VMM::get().allocPage();
    // printf("New Page is at: %x\n", newPage);

    // MallocHeap::init();

    // int* ptr = (int*)malloc(5000);
    // *ptr = 5;
    // printf("Some data I have at addr: %x is: %d\n", ptr, *ptr);
    // free(ptr);
    // ptr = new int(10);
    // printf("Some data I have at addr: %x is: %d\n", ptr, *ptr);

    // int* arr = new int[5];
    // for (int i = 0; i < 5; i++) arr[i] = i + 1;
    // for (int i = 0; i < 5; i++)
    //     printf("Array at idx %d is: %d\n", i, arr[i]);

    // MallocHeap::get().print();

    panic("Nothing to do");
}
