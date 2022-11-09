#include <kernel/tty.h>
#include <stdio.h>

void kernel_main(void) {
    terminal_initialize();
    printf("Hello, kernel World!\n");
}
