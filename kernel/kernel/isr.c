#include <kernel/isr.h>
#include <stdio.h>
#include <stdio.h>

isr_handler_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

extern void isr_handler(struct cpu_state cpu, uint32_t interrupt, struct stack_state stack) {
    if (interrupt_handlers[interrupt] != 0) {
        isr_handler_t handler = interrupt_handlers[interrupt];
        handler(cpu, interrupt, stack);
    }
    else {
        printf("Unhandled interrupt: %d\n", (int)interrupt);
    }
}
