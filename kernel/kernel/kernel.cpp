#include <kernel/tty.h>
#include <stdio.h>
#include <stdint-gcc.h>
#include <kernel/multiboot.h>
#include <kernel/common.h>
#include <demo/demo.h>

multiboot_info_t* mbd;
unsigned int grub_checkvalue;
extern uintptr_t kernel_start;
extern uintptr_t kernel_end;

extern "C" void kernel_early(multiboot_info_t * _mbd, unsigned int magic) {
    mbd = _mbd;
    grub_checkvalue = magic;
}

extern "C" void kernel_main(void) {
    terminal_initialize();
    printf("Hello, kernel World!\n");


    if (grub_checkvalue != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("invalid magic number!");
    }
    if (!(mbd->flags >> 6 & 0x1)) {
        panic("invalid memory map given by GRUB bootloader");
    }
    printf("System memory map:\n");
    for (int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt =
            (multiboot_memory_map_t*)(mbd->mmap_addr + i);

        printf("Start Addr: %u,%u | Length: %u,%u | Size: %u | Type: %u\n",
            mmmt->addr_high, mmmt->addr_low, mmmt->len_high, mmmt->len_low, mmmt->size, mmmt->type);

        if (mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
            /*
             * Do something with this memory block!
             * BE WARNED that some of memory shown as availiable is actually
             * actively being used by the kernel! You'll need to take that
             * into account before writing to memory!
             */
        }
    }
    printf("Kernel start memory:%u\n", &kernel_start);
    printf("Kernel end memory:%u\n", &kernel_end);

    panic("Nothing to do");
}
