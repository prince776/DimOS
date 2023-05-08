#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* kmalloc(uint64_t size);
void kfree(void* ptr);

#ifdef __cplusplus
}
#endif
