#include <stdio.h>

#if defined(__is_libk)
#include <kernel/debug.h>
#endif

int putchar(int ic) {
    char c = (char)ic;
#if defined(__is_libk)
    char str[2] = {c, 0};
    limineDebug(str, 1);
#else
    // TODO: Implement stdio and the write system call.
#endif
    return ic;
}
