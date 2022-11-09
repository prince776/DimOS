#include <kernel/tty.h>
#include <stdio.h>

#include <kernel/demo/demo.h>

void kernel_main(void) {
    terminal_initialize();
    demo_stack_smashing("Small string");
    demo_stack_smashing("a very big string string");
    printf("Hello, kernel World!\n");
}
