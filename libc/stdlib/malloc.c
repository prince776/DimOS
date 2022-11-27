#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>
#include <kernel/memory/kheap.h>

void* malloc(uint32_t size) {
#if defined(__is_libk)
    return kmalloc(size);
#else
    printf("Unimplemented malloc in libc\n");
#endif
    while (1) {}
    __builtin_unreachable();
}
