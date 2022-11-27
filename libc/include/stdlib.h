#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>
#include <stdint-gcc.h>
#ifdef __cplusplus
extern "C" {
#endif

    __attribute__((__noreturn__))
        void abort(void);
    void* malloc(uint32_t size);
    void free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif
