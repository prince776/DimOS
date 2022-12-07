// #include <kernel/tty.h>
// #include <stdio.h>
#include <stdint-gcc.h>
// #include <kernel/multiboot.h>
#include <kernel/common.h>
// #include <demo/demo.h>
// #include <kernel/memory/pmm.h>
// #include <kernel/memory/vmm.h>
// #include <kernel/memory/heap.h>
#include <stdlib.h>
#include <stddef.h>
#include <kernel/limine.h>
#include <stdio.h>
#include <kernel/debug.h>
#include <demo/demo.h>

// extern uintptr_t kernel_start;
// extern uintptr_t kernel_end;
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

static void done(void) {
    for (;;) {
        __asm__("hlt");
    }
}

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
    auto memArr = memmap_request.response->entries;
    auto memArrLen = memmap_request.response->entry_count;
    uint64_t total = 0;
    for (int i = 0; i < memArrLen; i++) {
        auto memEntry = memArr[i];
        printf("Memory entry %d: base: %x length: %x type: %x\n", i, memEntry->base, memEntry->length, memEntry->type);
        if (memEntry->type == 0)
            total = memEntry->base + memEntry->length;
    }

    auto kernelAddr = kerenel_address_request.response;
    printf("Kernel Addr-> Physical: %x Virtual: %x\n", kernelAddr->physical_base, kernelAddr->virtual_base);

    auto hhdmRes = hhdm_request.response;
    printf("HHDM offset: %x\n", hhdmRes->offset);
    // printf("--------------------------------------------------\n");
    // printf("Kernel memory range: [%x, %x)\n", &kernel_start, &kernel_end);
    // printf("Total RAM: [0, %x)\n", totalRamSize);
    // printf("Available ram range: [%x, %x)\n", availRamStart, availRamStart + availRamSize);
    // printf("--------------------------------------------------\n");

    // PMM::get().init(availRamStart + availRamSize, &kernel_end, availRamStart + availRamSize);

    // void* testFrame = PMM::get().allocFrame();
    // printf("Allocated memory: %x\n", testFrame);

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
