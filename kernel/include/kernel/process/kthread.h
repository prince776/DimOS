#pragma once
#include <stdlib.h>
#include <kernel/cpu.h>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/vector.hpp>
#include <kernel/isr.h>

namespace kernel {
    class Thread;

    struct ContextStack {
        uint64_t rsp = 0; // stack pointer
        cpu::GenRegisters genRegisters{};
        uint64_t rip = 0; // instruction pointer
        uint64_t rflags = 514; // duh
        Thread* controlBlock = nullptr;
        ContextStack() = default;
    };

    static constexpr uint64_t stackSize = 64 * 1024;
    struct StackSpace {
        char buffer[stackSize];
    };

    class Thread {
    public:
        uint64_t rsp = 0; // stack pointer
        uint64_t rip = 0; // instruction pointer
        cpu::GenRegisters genRegisters{};

        uint64_t rflags = 514;
        bool started = false;
        uint64_t id = 0;
        UniquePtr<StackSpace> stackMem;
        bool finished = false;

        Thread() = default;
        Thread(uint64_t newRip)
            : rsp(), rip(newRip), genRegisters() {
            static uint64_t currId = 1;
            id = currId++;

            stackMem = makeUnique<StackSpace>();
            rsp = (uint64_t)stackMem.get() + stackSize - 4096;
        }

        void copyContext(const ContextStack& context) {
            genRegisters = context.genRegisters;
            rsp = context.rsp;
            rip = context.rip;
            rflags = context.rflags;
        }

    private:
    };

}

extern "C" void scheduleKThread(kernel::ContextStack * prevContext);
extern "C" void return_from_yield(kernel::Thread * thread);
extern "C" void return_from_interrupt(kernel::Thread * thread);
namespace kernel {
    Thread& thisThread();
}

void premptiveScheduler(ISRFrame* isrFrame);
