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
