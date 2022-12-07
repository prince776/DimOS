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
