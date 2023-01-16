#pragma once
#include <stdint-gcc.h>
#include <kernel/cpp/vector.hpp>
#include <kernel/process/kthread.h>

class MutexLock {
private:
    // flag is the mutex value, lock is guard to access flag and queue
    int64_t flag = 0, lock = 0;
    Vector<uint64_t> queue; // a vector named queue that will be used as stack
public:
    MutexLock() = default;

    void acquireLock() {
        // we have this because even after coming from park, there's not guarantee flag is free
        // it's like a condition variable, just a signal
        // reason: this thread was unparked, but then before it could be scheduled a new thread was spawned
        // which acquired this mutex
        while (true) {
            while (__sync_bool_compare_and_swap(&lock, 0, 1) == 0)
                continue;
            if (!flag) {
                flag = 1;
                lock = 0;
                break;
            }
            else {
                queue.push_back(kernel::thisThread().id);
                kernel::setpark(); // Now can't be interrupted till we yield
                lock = 0;
                kernel::park();
            }
        }
    }
    void releaseLock() {
        while (__sync_bool_compare_and_swap(&lock, 0, 1) == 0)
            continue;
        if (!queue.size()) {
            flag = 0;
        }
        else {
            auto next = queue.back();
            queue.pop_back();
            kernel::unpark(kernel::getThread(next));
        }
        lock = 0;
    }
};
