#ifndef _KERNEL_KHEAP_H
#define _KERNEL_KHEAP_H

#include <stdint-gcc.h>

// USED ONLY FOR PAGING DATA ALLOCATION, which is never freed
// frame state tracker (bitmap)
// page directory + page tables
uint32_t kmalloc_a(uint32_t); // page aligned
uint32_t kmalloc_p(uint32_t, uint32_t*); // returns a physical addr
uint32_t kmalloc_ap(uint32_t, uint32_t*); // page aligned & returns a physical address
uint32_t kmalloc(uint32_t); // vanilla

#endif