// ref: https://littleosbook.github.io/#interrupts-and-input
// Probably shouldn't be here, because registers are arch specefic
// when x86_64 would be added, this should be improved
#pragma once
#include <stdint-gcc.h>

struct ISRFrame {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t isrNumber;
    uint64_t errorCode;

    // Interrupt stack frame
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

typedef void (*isr_handler_t)(ISRFrame* frame);
void register_interrupt_handler(uint8_t n, isr_handler_t handler);
// void page_fault_handler(cpu_state cpu, uint32_t interrupt, stack_state stack);
