#include <stdint-gcc.h>
#include "descriptor_tables.h"

static void init_gdt();
// static void init_idt();

extern "C" void init_descriptor_tables()
{
    // asm volatile("cli");
    init_gdt();
    // init_idt();
}

// -------------- Stuff related to GDT --------------

extern "C" void gdt_flush(uint64_t);
static void gdt_set_gate(int32_t, uint32_t, uint32_t, uint8_t, uint8_t);

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

static void GDT_setGate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdtEntries[num].baseLow = (base & 0xFFFF);
    gdtEntries[num].baseMiddle = (base >> 16) & 0xFF;
    gdtEntries[num].baseHigh = (base >> 24) & 0xFF;

    gdtEntries[num].limitLow = (limit & 0xFFFF);
    gdtEntries[num].granularity = (limit >> 16) & 0x0F;

    gdtEntries[num].granularity |= gran & 0xF0;
    gdtEntries[num].access = access;
}

// -------------- Stuff related to IDT --------------

// // These extern "C" directives let us access the addresses of our ASM ISR handlers.
// extern "C" void isr0();
// extern "C" void isr1();
// extern "C" void isr2();
// extern "C" void isr3();
// extern "C" void isr4();
// extern "C" void isr5();
// extern "C" void isr6();
// extern "C" void isr7();
// extern "C" void isr8();
// extern "C" void isr9();
// extern "C" void isr10();
// extern "C" void isr11();
// extern "C" void isr12();
// extern "C" void isr13();
// extern "C" void isr14();
// extern "C" void isr15();
// extern "C" void isr16();
// extern "C" void isr17();
// extern "C" void isr18();
// extern "C" void isr19();
// extern "C" void isr20();
// extern "C" void isr21();
// extern "C" void isr22();
// extern "C" void isr23();
// extern "C" void isr24();
// extern "C" void isr25();
// extern "C" void isr26();
// extern "C" void isr27();
// extern "C" void isr28();
// extern "C" void isr29();
// extern "C" void isr30();
// extern "C" void isr31();
// extern "C" void idt_flush(uint32_t);

// static void idt_set_gate(uint8_t, uint32_t, uint16_t, uint8_t);

// __attribute__((aligned(0x10)))
// idt_entry_t idt_entries[256];
// idt_ptr_t   idt_ptr;

// static void init_idt()
// {
//     idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
//     idt_ptr.base = (uint32_t)&idt_entries;

//     memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

//     idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
//     idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
//     idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
//     idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
//     idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
//     idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
//     idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
//     idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
//     idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
//     idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
//     idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
//     idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
//     idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
//     idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
//     idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
//     idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
//     idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
//     idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
//     idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
//     idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
//     idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
//     idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
//     idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
//     idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
//     idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
//     idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
//     idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
//     idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
//     idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
//     idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
//     idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
//     idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

//     idt_flush((uint32_t)&idt_ptr);
// }

// static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
// {
//     idt_entries[num].base_lo = base & 0xFFFF;
//     idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

//     idt_entries[num].sel = sel;
//     idt_entries[num].always0 = 0;
//     // We must uncomment the OR below when we get to using user-mode.
//     // It sets the interrupt gate's privilege level to 3.
//     idt_entries[num].flags = flags /* | 0x60 */;
// }
