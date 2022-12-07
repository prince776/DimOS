// ref: https://littleosbook.github.io/#interrupts-and-input
// Probably shouldn't be here, because registers are arch specefic
// when x86_64 would be added, this should be improved
#pragma once
#include <stdint-gcc.h>

struct ISRFrame {
    struct {
        uint64_t    cr4;
        uint64_t    cr3;
        uint64_t    cr2;
        uint64_t    cr0;
    } control_registers;

    struct {
        uint64_t    rdi;
        uint64_t    rsi;
        uint64_t    rdx;
        uint64_t    rcx;
        uint64_t    rbx;
        uint64_t    rax;
    } general_registers;

    struct {
        uint64_t    rbp;
        uint64_t    vector;
        uint64_t    error_code;
        uint64_t    rip;
        uint64_t    cs;
        uint64_t    rflags;
        uint64_t    rsp;
        uint64_t    dss;
    } base_frame;
};

typedef void (*isr_handler_t)(ISRFrame* frame);
void register_interrupt_handler(uint8_t n, isr_handler_t handler);
// void page_fault_handler(cpu_state cpu, uint32_t interrupt, stack_state stack);
