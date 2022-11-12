#include <kernel/tty.h>
#include <stdio.h>
#include <stdint-gcc.h>
#include <demo/demo.h>
#include "../arch/i386/paging.h"

void kernel_main(void) {
    terminal_initialize();
    initialise_paging();

    // asm volatile ("int $0x4");
    printf("Hello, kernel World!\n");

    // uint32_t* ptr = (uint32_t*)0xA0000000;
    // uint32_t do_page_fault = *ptr;

    while (1) {}
}
