#include <kernel/concurrency/primitives.h>

void MutexLock::acquireLock() {
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

void MutexLock::releaseLock() {
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

void ConditionVariable::wait(MutexLock& mutex) {
    while (__sync_val_compare_and_swap(&lock, 0, 1) == 1)
        continue;
    mutex.releaseLock(); // can be before acquiring lock
    queue.push_back(kernel::thisThread().id);
    kernel::setpark();
    lock = 0;
    kernel::park();
}

void ConditionVariable::notifyOne() {
    while (__sync_val_compare_and_swap(&lock, 0, 1) == 1)
        continue;
    if (queue.size()) {
        auto next = queue.back();
        queue.pop_back();
        kernel::unpark(kernel::getThread(next));
    }
    lock = 0;
}

void ConditionVariable::notifyAll() {
    while (__sync_val_compare_and_swap(&lock, 0, 1) == 1)
        continue;
    while (queue.size()) {
        auto next = queue.back();
        queue.pop_back();
        kernel::unpark(kernel::getThread(next));
    }
    lock = 0;
}

void Sempahore::wait() {
    lock.acquireLock();
    while (value <= 0)
        condition.wait(lock);
    value--;
    lock.releaseLock();
}

void Sempahore::post() {
    lock.acquireLock();
    value++;
    condition.notifyOne();
    lock.releaseLock();
}
