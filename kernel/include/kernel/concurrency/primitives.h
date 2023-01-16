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
            while (__sync_val_compare_and_swap(&lock, 0, 1) == 1)
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
        while (__sync_val_compare_and_swap(&lock, 0, 1) == 1)
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


class ConditionVariable {
private:
    int64_t lock = 0;
    Vector<uint64_t> queue; // a vector named queue that will be used as stack
public:
    ConditionVariable() = default;

    // PreCondition: This mutex lock must be held
    void wait(MutexLock& mutex) {
        while (__sync_val_compare_and_swap(&lock, 0, 1) == 1)
            continue;
        mutex.releaseLock(); // can be before acquiring lock
        queue.push_back(kernel::thisThread().id);
        kernel::setpark();
        lock = 0;
        kernel::park();
    }

    void notifyOne() {
        while (__sync_val_compare_and_swap(&lock, 0, 1) == 1)
            continue;
        if (queue.size()) {
            auto next = queue.back();
            queue.pop_back();
            kernel::unpark(kernel::getThread(next));
        }
        lock = 0;
    }

    void notifyAll() {
        while (__sync_val_compare_and_swap(&lock, 0, 1) == 1)
            continue;
        while (queue.size()) {
            auto next = queue.back();
            queue.pop_back();
            kernel::unpark(kernel::getThread(next));
        }
        lock = 0;
    }
};

// Intentionally made using Mutex and ConditionVariable, so that if
// there's a big in concurrency, only they need to be fixed.
class Sempahore {
private:
    int32_t value;
    MutexLock lock;
    ConditionVariable condition;
public:
    Sempahore(int value)
        : value(value) {}

    void wait() {
        lock.acquireLock();
        while (value <= 0)
            condition.wait(lock);
        value--;
        lock.releaseLock();
    }

    void post() {
        lock.acquireLock();
        value++;
        condition.notifyOne();
        lock.releaseLock();
    }
};
