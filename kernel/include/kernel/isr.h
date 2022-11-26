// ref: https://littleosbook.github.io/#interrupts-and-input
// Probably shouldn't be here, because registers are arch specefic
// when x86_64 would be added, this should be improved
#pragma once
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


typedef void (*interrupt_handler_t)(cpu_state cpu, uint32_t interrupt, stack_state stack);

void register_interrupt_handler(uint8_t n, interrupt_handler_t handler);

void page_fault_handler(cpu_state cpu, uint32_t interrupt, stack_state stack);

constexpr int IRQ0 = 32;
constexpr int IRQ1 = 33;
constexpr int IRQ2 = 34;
constexpr int IRQ3 = 35;
constexpr int IRQ4 = 36;
constexpr int IRQ5 = 37;
constexpr int IRQ6 = 38;
constexpr int IRQ7 = 39;
constexpr int IRQ8 = 40;
constexpr int IRQ9 = 41;
constexpr int IRQ10 = 42;
constexpr int IRQ11 = 43;
constexpr int IRQ12 = 44;
constexpr int IRQ13 = 45;
constexpr int IRQ14 = 46;
constexpr int IRQ15 = 47;
