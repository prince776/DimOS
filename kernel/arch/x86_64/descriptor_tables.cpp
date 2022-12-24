#include <stdint-gcc.h>
#include "descriptor_tables.h"
#include <kernel/devices/pic.h>
#include <kernel/x64.h>

static void init_gdt();
static void init_idt();

extern "C" void init_descriptor_tables()
{
    init_gdt();
    init_idt();
    asm("sti");
}

// -------------- Stuff related to GDT --------------

extern "C" void gdt_flush(uint64_t);

GDTPtr   gdtPtr;
GDTEntry gdtEntries[9];

static void init_gdt()
{
    gdtEntries[0] = { 0, 0, 0, 0, 0, 0 }; // null
    gdtEntries[1] = { 0xffff, 0, 0, 0x9a, 0x80, 0 }; // 16-bit code
    gdtEntries[2] = { 0xffff, 0, 0, 0x92, 0x80, 0 }; // 16-bit data
    gdtEntries[3] = { 0xffff, 0, 0, 0x9a, 0xcf, 0 }; // 32-bit code
    gdtEntries[4] = { 0xffff, 0, 0, 0x92, 0xcf, 0 }; // 32-bit data
    gdtEntries[5] = { 0, 0, 0, 0x9a, 0xa2, 0 }; // 64-bit code
    gdtEntries[6] = { 0, 0, 0, 0x92, 0xa0, 0 }; // 64-bit data
    gdtEntries[7] = { 0, 0, 0, 0xF2, 0, 0 }; // user data
    gdtEntries[8] = { 0, 0, 0, 0xFA, 0x20, 0 }; // user code

    gdtPtr.limit = sizeof(GDTEntry) * 9 - 1;
    gdtPtr.base = (uint64_t)&gdtEntries;
    gdt_flush((uint64_t)&gdtPtr);
}

// -------------- Stuff related to IDT --------------

extern void* isr_stub_table[];
extern "C" void idt_reload(IDTPtr * idtr);

static void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

constexpr int IDT_MAX_DESCRIPTORS = 256;
__attribute__((aligned(0x10))) IDTEntry idtEntries[IDT_MAX_DESCRIPTORS];
IDTPtr idtPtr;

static void init_idt()
{
    idtPtr.base = (uint64_t)&idtEntries[0];
    idtPtr.limit = (uint16_t)(sizeof(IDTEntry) * IDT_MAX_DESCRIPTORS - 1);

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E); // This flag means, 64 bit interrupt gate(not trap) + present
        // this means interrupts will be disabled in these ISRs
    }
    pic::remap();
    for (uint8_t vector = pic::PIC1Offset; vector < pic::PIC2End; vector++) { // Set ISRs for remapped PIC Interrupts
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }

    idt_reload(&idtPtr);
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    auto& descriptor = idtEntries[vector];

    descriptor.isrLow = (uint64_t)isr & 0xFFFF;
    descriptor.kernelCS = 0x28;
    descriptor.ist = 0;
    descriptor.attributes = flags;
    descriptor.isrMid = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor.isrHigh = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor.reserved = 0;
}
