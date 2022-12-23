#pragma once
#include <stdint-gcc.h>

namespace cpu {
    struct GenRegisters {
        uint64_t r15 = 0;
        uint64_t r14 = 0;
        uint64_t r13 = 0;
        uint64_t r12 = 0;
        uint64_t r11 = 0;
        uint64_t r10 = 0;
        uint64_t r9 = 0;
        uint64_t r8 = 0;
        uint64_t rbp = 0;
        uint64_t rdi = 0;
        uint64_t rsi = 0;
        uint64_t rdx = 0;
        uint64_t rcx = 0;
        uint64_t rbx = 0;
        uint64_t rax = 0;
    };

    struct InterruptFrame {
        uint64_t rip;
        uint64_t cs;
        uint64_t rflags;
        uint64_t rsp;
        uint64_t ss;
    };
}
