#include <stddef.h>
#include <stdlib.h>
#include <stdint-gcc.h>

void* operator new(size_t size)
{
    return malloc(size);
}

void* operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void* p)
{
    free(p);
}

void operator delete(void* p, unsigned long x)
{
    free(p);
}

void operator delete[](void* p)
{
    free(p);
}

void operator delete[](void* p, unsigned long x)
{
    free(p);
}

///////// Placement new/delete //////////
inline void* operator new(size_t, void* p)     throw() { return p; }
inline void* operator new[](size_t, void* p)   throw() { return p; }
inline void  operator delete  (void*, void*) throw() { };
inline void  operator delete[](void*, void*) throw() {};