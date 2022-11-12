// ref: https://littleosbook.github.io/#interrupts-and-input
// Probably shouldn't be here, because registers are arch specefic
// when x86_64 would be added, this should be improved
#ifndef _KERNEL_ISR_H
#define _KERNEL_ISR_H

#include <stdint-gcc.h>

struct cpu_state {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
} __attribute__((packed));

struct stack_state {
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((packed));

typedef void (*isr_handler_t)(struct cpu_state cpu, uint32_t interrupt, struct stack_state stack);
void register_interrupt_handler(uint8_t n, isr_handler_t handler);

#endif