#include <kernel/isr.h>
#include <stdio.h>
#include <stdio.h>

extern void isr_handler(struct cpu_state cpu, uint32_t interrupt, struct stack_state stack) {
    printf("Interrupt occured: %d\n", (int)interrupt);
}
