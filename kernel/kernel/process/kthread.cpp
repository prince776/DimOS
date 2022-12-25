#include <kernel/process/kthread.h>
#include <kernel/cpp/vector.hpp>
#include <kernel/cpp/utility.hpp>
#include <stddef.h>
#include <stdio.h>

extern Vector<kernel::Thread>* kthreadsPtr;
extern size_t currKThreadIdx;

extern "C" void scheduleKThread(kernel::ContextStack * prevContext) {
    auto* prevThread = prevContext->controlBlock;
    auto& kthreads = *kthreadsPtr;
    size_t prevIndex = kthreads.size();
    for (size_t i = 0; i < kthreads.size(); i++) {
        if (kthreads[i].id == prevThread->id) {
            prevIndex = i;
            break;
        }
    }
    if (!prevThread->started) {
        prevThread->started = true;
    }
    bool validIdx = prevIndex < kthreads.size();
    if (!prevThread->finished && validIdx) {
        prevThread->copyContext(*prevContext);
    }

    if (prevThread->finished && validIdx) { // remove the thread
        bool swapToEnd = prevIndex != (kthreads.size() - 1);
        if (swapToEnd) {
            swap(kthreads[prevIndex], kthreads[kthreads.size() - 1]);
        }
        kthreads.pop_back();
        // TODO: isn't exactly round robin, but that's fine for now, since it's not premptive, ig?
    }
    if (!kthreads.size()) {
        panic("No kernel threads left\n");
    }
    currKThreadIdx = (prevIndex + 1) % kthreads.size();
    auto& nextThread = kthreads[currKThreadIdx];
    // return_from_yield(&kthreads[currKThreadIdx]);
    return_from_interrupt(&nextThread);
}

namespace kernel {
    Thread& thisThread() {
        return (*kthreadsPtr)[currKThreadIdx];
    }
}

void premptiveScheduler(ISRFrame* isrFrame) {
    kernel::ContextStack prevContext;
    prevContext.controlBlock = &kernel::thisThread();
    prevContext.genRegisters = isrFrame->genRegisters;
    prevContext.rip = isrFrame->interruptFrame.rip;
    prevContext.rsp = isrFrame->interruptFrame.rsp;
    // if (kernel::thisThread().started) {
    prevContext.rflags = isrFrame->interruptFrame.rflags;
    // }
    scheduleKThread(&prevContext);
}

