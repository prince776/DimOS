#include <stdio.h>


void panic(const char* str) {
    printf("Panic: %s\n", str);
    while (1);
}