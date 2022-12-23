// ref: https://littleosbook.github.io/#interrupts-and-input
// Probably shouldn't be here, because registers are arch specefic
// when x86_64 would be added, this should be improved
#pragma once
#include <stdint-gcc.h>
#include <kernel/cpu.h>

struct ISRFrame {
    cpu::GenRegisters genRegisters;

    uint64_t isrNumber;
    uint64_t errorCode;

    cpu::InterruptFrame interruptFrame;
};

typedef void (*isr_handler_t)(ISRFrame* frame);
void registerInterruptHandler(uint8_t n, isr_handler_t handler);
void pageFaultHanlder(ISRFrame* frame);
