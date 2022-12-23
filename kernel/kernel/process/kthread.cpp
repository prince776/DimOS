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

    if (!prevThread->finished && prevIndex < kthreads.size()) {
        prevThread->copyContext(*prevContext);
    }

    if (prevThread->finished && prevIndex < kthreads.size()) { // remove the thread
        bool swapToEnd = prevIndex != kthreads.size() - 1;
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
    switch_kthread(&kthreads[currKThreadIdx]);
}

namespace kernel {
    Thread& thisThread() {
        return (*kthreadsPtr)[currKThreadIdx];
    }
}
