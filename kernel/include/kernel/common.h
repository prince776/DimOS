#pragma once
#include <stdio.h>
#include <stdint-gcc.h>

#ifdef __cplusplus
extern "C"
{
#endif
    void panic(const char* str);
    void outb(uint16_t port, uint8_t value);
#ifdef __cplusplus
}
#endif
