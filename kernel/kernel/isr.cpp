#include <kernel/isr.h>
#include <stdio.h>
#include <stdio.h>
#include <kernel/common.h>

isr_handler_t interrupt_handlers[256];

extern "C" void isr_handler(ISRFrame * frame) {
    auto vector = frame->base_frame.vector;
    if (interrupt_handlers[vector] != 0) {
        isr_handler_t handler = interrupt_handlers[vector];
        handler(frame);
    }
    else {
        printf("Unhandled interrupt: %x\n", vector);
    }
}

void register_interrupt_handler(uint8_t n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

// void page_fault_handler(cpu_state cpu, uint32_t interrupt, stack_state stack) {
//     // A page fault has occurred.
//     // The faulting address is stored in the CR2 register.
//     uint32_t faulting_address;
//     asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

//     // The error code gives us details of what happened.
//     int present = !(stack.error_code & 0x1); // Page not present
//     int rw = stack.error_code & 0x2;           // Write operation?
//     int us = stack.error_code & 0x4;           // Processor was in user-mode?
//     int reserved = stack.error_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
//     int id = stack.error_code & 0x10;          // Caused by an instruction fetch?

//     // Output an error message.
//     printf("Page fault! ( ");
//     if (present) { printf("present "); }
//     if (rw) { printf("read-only "); }
//     if (us) { printf("user-mode "); }
//     if (reserved) { printf("reserved "); }
//     printf(") at addr:%d\n", faulting_address);

//     panic("---------------CAN'T PROCEED AFTER PAGE FAULT---------------");
// }