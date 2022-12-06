#include <stdio.h>

#if defined(__is_libk)
#endif

int putchar(int ic) {
#if defined(__is_libk)
#else
	// TODO: Implement stdio and the write system call.
#endif
	return ic;
}
