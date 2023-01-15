#include <kernel/process/kthread.h>
#include <kernel/cpp/vector.hpp>
#include <kernel/cpp/utility.hpp>
#include <stddef.h>
#include <stdio.h>

extern Vector<kernel::Thread>* kthreadsPtr;
extern size_t currKThreadIdx;

extern "C" void scheduleKernelThread(Vector<kernel::Thread> &threads, kernel::Thread & prevThread) {
    printf("SWITCHING CONTEXT\n");
    size_t prevIndex = threads.size();
    for (size_t i = 0; i < threads.size(); i++) {
        if (threads[i].id == prevThread.id) {
            prevIndex = i;
            break;
        }
    }
    prevThread.started = true;
    bool validIdx = prevIndex < threads.size();

    if (prevThread.finished && validIdx) { // remove the thread
        bool swapToEnd = prevIndex != (threads.size() - 1);
        if (swapToEnd) {
            swap(threads[prevIndex], threads[threads.size() - 1]);
        }
        threads.pop_back();
        // TODO: isn't exactly round robin, but that's fine for now, since it's not premptive, ig?
    }
    if (!threads.size()) {
        panic("No kernel threads left\n");
    }
    // TODO: shared var update
    currKThreadIdx = (prevIndex + 1) % threads.size();
    auto& nextThread = threads[currKThreadIdx];
    // return_from_yield(&kthreads[currKThreadIdx]);
    return_from_interrupt(&nextThread);
}

namespace kernel {
    Thread& thisThread() {
        return (*kthreadsPtr)[currKThreadIdx];
    }
}

void premptiveScheduler(ISRFrame* isrFrame) {
    auto& prevThread = kernel::thisThread();
    prevThread.copyFromISRFrame(*isrFrame);
    scheduleKernelThread(*kthreadsPtr, prevThread);
}

