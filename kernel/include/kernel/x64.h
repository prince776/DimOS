#pragma once
#include <stdint-gcc.h>

// sends data to a IO port
inline void io_outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// receives data from a IO port
inline uint8_t io_inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0"
        : "=a"(ret)
        : "Nd"(port));
    return ret;
}

// another I/0 cycle on an unused, CPU-speed independent port
inline void io_wait(void)
{
    io_inb(0x80);
}
