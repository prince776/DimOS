#pragma once
#include <stdint-gcc.h>
#include <string.h>

struct GDTEntry {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t  baseMiddle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  baseHigh;
} __attribute__((packed));

struct GDTPtr
{
    uint16_t limit;
    uint64_t base;
}
__attribute__((packed));

struct IDTEntry {
    uint16_t    isrLow;      // The lower 16 bits of the ISR's address
    uint16_t    kernelCS;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isrMid;      // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t    isrHigh;     // The higher 32 bits of the ISR's address
    uint32_t    reserved;     // Set to zero
} __attribute__((packed));

struct IDTPtr {
    uint16_t	limit;
    uint64_t	base;
} __attribute__((packed));
