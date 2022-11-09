#include <string.h>
#include <stdio.h>

void demo_stack_smashing(const char* str) {
    char buffer[16];
    strcpy(buffer, str);
    printf("String copied: %s\n", buffer);
}