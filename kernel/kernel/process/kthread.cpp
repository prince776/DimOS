#include <kernel/process/kthread.h>
#include <kernel/cpp/vector.hpp>
#include <kernel/cpp/utility.hpp>
#include <kernel/process/scheduler.h>
#include <stddef.h>
#include <stdio.h>
#include <kernel/filesystem/vfs.h>

extern Vector<kernel::Thread> kthreads;
extern size_t currKThreadIdx;

extern "C" void scheduleKernelThread(Vector<kernel::Thread> &threads, kernel::Thread & prevThread) {
    printf("SWITCHING CONTEXT\n");
    auto contextSwitchInfo = prevThread.contextSwitchInfo;
    prevThread.resetContextSwitchInfo();
    auto nextIdx = RRScheduler<kernel::Thread>::get().getNext(threads, prevThread);
    if (nextIdx == threads.size()) {
        panic("No kernel threads left");
    }
    // TODO: shared var update
    currKThreadIdx = nextIdx;
    auto& nextThread = threads[currKThreadIdx];
    return_from_interrupt(&nextThread, &contextSwitchInfo);
}
extern vfs::VFS globalVFS;

namespace kernel {

    Thread::Thread(void(*func)(void))
        : runFunc(func) {
        static uint64_t currId = 1;
        id = currId++;

        stackMem = makeUnique<StackSpace>();
        execContext.rip = (uint64_t)&threadRunWrapper;
        execContext.rsp = (uint64_t)stackMem.get() + stackSize - 4096;

        resetContextSwitchInfo();

        String<> stdinPath = String<>("/proc/stdin_") + stoi(id);
        String<> stdoutPath = String<>("/proc/stdout_") + stoi(id);

        fileDescriptors.push_back(FileDescriptor(globalVFS.mkfile(stdinPath)));
        fileDescriptors.push_back(FileDescriptor(globalVFS.mkfile(stdoutPath)));
    }

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

namespace kernel {

    bool  FileDescriptor::canReadXbytes(int x) {
        int remainingSize = fileNode->resource.size - (int)readOffset;

        return remainingSize >= x;
    }

    int FileDescriptor::read(uint32_t limit, uint8_t* buffer) {
        int bytesRead = fileNode->read(readOffset, limit, buffer);

        readOffset += bytesRead;
        return bytesRead;
    }

    int FileDescriptor::write(uint32_t limit, uint8_t* buffer) {
        int bytesWritten = fileNode->write(writeOffset, limit, buffer);
        writeOffset += bytesWritten;
        return bytesWritten;
    }
}
