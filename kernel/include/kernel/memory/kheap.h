#pragma once
#include <stdint-gcc.h>

#ifdef __cplusplus
extern "C" {
#endif

    void* kmalloc(uint32_t size);
    void kfree(void* ptr);

#ifdef __cplusplus
}
#endif