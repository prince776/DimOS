#include <kernel/memory/kheap.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void free(void* ptr) {
#if defined(__is_libk)
    return kfree(ptr);
#else
    printf("Unimplemented free in libc\n");
#endif
    while (1) {
    }
    __builtin_unreachable();
}
