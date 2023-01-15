#pragma once
#include <stdlib.h>
#include <kernel/cpu.h>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/vector.hpp>
#include <kernel/isr.h>

void kthreadRunWrapper();

namespace kernel {
    class Thread;

    struct ExecContext {
        uint64_t rsp = 0; // stack pointer
        uint64_t rip = 0; // instruction pointer
        cpu::GenRegisters genRegisters{};
        uint64_t rflags = 514; // duh

        ExecContext() = default;
        ExecContext(uint64_t newRip): rip(newRip) {}
    };

    static constexpr uint64_t stackSize = 64 * 1024;
    struct StackSpace {
        char buffer[stackSize];
    };

    class Thread {
    public:
        ExecContext execContext;

        bool started = false;
        uint64_t id = 0;
        UniquePtr<StackSpace> stackMem;
        bool finished = false;
        void(*runFunc)(void);

        Thread() = default;

        Thread(void(*func)(void))
            : runFunc(func) {
            static uint64_t currId = 1;
            id = currId++;

            stackMem = makeUnique<StackSpace>();
            execContext.rip = (uint64_t)&kthreadRunWrapper;
            execContext.rsp = (uint64_t)stackMem.get() + stackSize - 4096;
        }

        void copyFromISRFrame(const ISRFrame& context) {
            execContext.genRegisters = context.genRegisters;
            execContext.rflags = context.interruptFrame.rflags;
            execContext.rsp = context.interruptFrame.rsp;
            execContext.rip = context.interruptFrame.rip;
        }

    };

}

extern "C" void scheduleKernelThread(Vector<kernel::Thread> &threads, kernel::Thread & prevThread);
extern "C" void return_from_interrupt(kernel::Thread * thread);
namespace kernel {
    Thread& thisThread();
}

void premptiveScheduler(ISRFrame* isrFrame);

inline void yield() {
    __asm__ volatile ("int $0x20");
}

inline void kthreadRunWrapper() {
    kernel::thisThread().runFunc();
    kernel::thisThread().finished = true;
    yield();
}
