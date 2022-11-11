#include <kernel/tty.h>
#include <stdio.h>

#include <kernel/demo/demo.h>

void kernel_main(void) {
    terminal_initialize();
    printf("Hello, kernel World!\n");
    asm volatile ("int $0x4");

    while (1) {}
}
