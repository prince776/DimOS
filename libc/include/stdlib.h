#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <stdint.h>
#include <sys/cdefs.h>
#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__)) void abort(void);
void* malloc(uint64_t size);
void free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif
