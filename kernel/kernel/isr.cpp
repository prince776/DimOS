#include <kernel/isr.h>
#include <stdio.h>
#include <stdio.h>
#include <kernel/common.h>
#include <kernel/devices/pic.h>
#include <kernel/devices/pit.h>

isr_handler_t interrupt_handlers[256];

extern "C" void isr_handler(ISRFrame * frame) {
    auto isrNum = frame->isrNumber;
    if (isrNum < 256 && interrupt_handlers[isrNum] != 0) {
        isr_handler_t handler = interrupt_handlers[isrNum];
        handler(frame);
    }
    else {
        printf("Unhandled interrupt: %x\n", isrNum);

        { // code to stall execution a bit to properly see what chain of interrupts happen
            volatile int sum = 0;
            for (int i = 0; i < 100000000; i++) {
                sum |= i;
                if (i % 10000000 == 0) printf(".");
            }
            printf("did some summing: %d\n", sum);
        }
    }


    if (isrNum >= pic::PIC1Offset && isrNum < pic::PIC2End) {
        // printf("singaling EOI for isr: %d\n", isrNum);
        pic::signalEOI(isrNum);
    }
}

void registerInterruptHandler(uint8_t n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

void pageFaultHanlder(ISRFrame* frame) {
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint64_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // The error code gives us details of what happened.
    int present = !(frame->errorCode & 0x1); // Page not present
    int rw = frame->errorCode & 0x2;           // Write operation?
    int us = frame->errorCode & 0x4;           // Processor was in user-mode?
    int reserved = frame->errorCode & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = frame->errorCode & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
    printf("Page fault! ( ");
    if (present) { printf("present "); }
    if (rw) { printf("read-only "); }
    if (us) { printf("user-mode "); }
    if (reserved) { printf("reserved "); }
    printf(") at addr:%x\n", faulting_address);

    panic("---------------CAN'T PROCEED AFTER PAGE FAULT---------------");
}
