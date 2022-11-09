#include <string.h>

void* strcpy(char* restrict dest, const char* restrict src) {
    memcpy(dest, src, strlen(src));
}
