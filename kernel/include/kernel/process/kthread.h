#pragma once
#include <stdlib.h>
#include <kernel/cpu.h>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/vector.hpp>
#include <kernel/isr.h>
#include <kernel/isr.h>

enum class TaskState {
    NOT_STARTED = 0,
    RUNNING = 1,
    SLEEPING = 2,
    BLOCKED = 4,
    COMPLETED = 8,
    DO_NOT_DISTURB = 16,
};

inline bool isRunnableState(TaskState state) {
    return state == TaskState::NOT_STARTED || state == TaskState::RUNNING;
}

namespace kernel {
    void threadRunWrapper();

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

    struct ContextSwitchInfo {
        enum class Method {
            NORMALLY_INTERRUPTED = 0,
            YIELDED = 1,
        };
        Method method = Method::NORMALLY_INTERRUPTED;
    };

    class Thread {
    public:
        ExecContext execContext;
        ContextSwitchInfo contextSwitchInfo;

        uint64_t id = 0;
        UniquePtr<StackSpace> stackMem;

        TaskState state = TaskState::NOT_STARTED;
        void(*runFunc)(void);

        Thread() = default;

        Thread(void(*func)(void))
            : runFunc(func) {
            static uint64_t currId = 1;
            id = currId++;

            stackMem = makeUnique<StackSpace>();
            execContext.rip = (uint64_t)&threadRunWrapper;
            execContext.rsp = (uint64_t)stackMem.get() + stackSize - 4096;

            resetContextSwitchInfo();
        }

        void copyFromISRFrame(const ISRFrame& context) {
            execContext.genRegisters = context.genRegisters;
            execContext.rflags = context.interruptFrame.rflags;
            execContext.rsp = context.interruptFrame.rsp;
            execContext.rip = context.interruptFrame.rip;
        }

        void resetContextSwitchInfo() {
            contextSwitchInfo.method = ContextSwitchInfo::Method::NORMALLY_INTERRUPTED;
        }
    };

    Thread& thisThread();
    Thread& getThread(size_t idx);

    inline void yield() {
        thisThread().contextSwitchInfo.method = kernel::ContextSwitchInfo::Method::YIELDED;
        __asm__ volatile ("int $0x20");
    }

    inline void threadRunWrapper() {
        thisThread().runFunc();
        thisThread().state = TaskState::COMPLETED;
        yield();
    }

    inline void setpark() {
        thisThread().state = TaskState::DO_NOT_DISTURB;
    }

    inline void park() {
        thisThread().state = TaskState::SLEEPING;
        yield();
    }

    inline void unpark(kernel::Thread& thread) {
        thread.state = TaskState::RUNNING;
    }
}

extern "C" void scheduleKernelThread(Vector<kernel::Thread> &threads, kernel::Thread & prevThread);
extern "C" void return_from_interrupt(kernel::Thread * thread, kernel::ContextSwitchInfo*);

void premptiveScheduler(ISRFrame* isrFrame);
