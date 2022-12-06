#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>
// #include <kernel/memory/kheap.h>

void free(void* ptr) {
#if defined(__is_libk)
    // return kfree(ptr);
#else
    printf("Unimplemented free in libc\n");
#endif
    while (1) {}
    __builtin_unreachable();
}
