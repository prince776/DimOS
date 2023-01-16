#include <kernel/process/kthread.h>
#include <kernel/cpp/vector.hpp>
#include <kernel/cpp/utility.hpp>
#include <kernel/process/scheduler.h>
#include <stddef.h>
#include <stdio.h>

extern Vector<kernel::Thread> kthreads;
extern size_t currKThreadIdx;

extern "C" void scheduleKernelThread(Vector<kernel::Thread> &threads, kernel::Thread & prevThread) {
    printf("SWITCHING CONTEXT\n");
    auto nextIdx = RRScheduler<kernel::Thread>::get().getNext(threads, prevThread);
    if (nextIdx == threads.size()) {
        panic("No kernel threads left");
    }
    // TODO: shared var update
    currKThreadIdx = nextIdx;
    auto& nextThread = threads[currKThreadIdx];
    return_from_interrupt(&nextThread);
}

namespace kernel {
    Thread& thisThread() {
        return kthreads[currKThreadIdx];
    }

    Thread& getThread(size_t id) {
        for (auto& thread : kthreads) {
            if (thread.id == id) {
                return thread;
            }
        }
        panic("requested invalid thread");
        __builtin_unreachable();
    }
}

void premptiveScheduler(ISRFrame* isrFrame) {
    auto& prevThread = kernel::thisThread();
    prevThread.copyFromISRFrame(*isrFrame);
    scheduleKernelThread(kthreads, prevThread);
}

