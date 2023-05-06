#pragma once
#include <kernel/concurrency/primitives.h>

class ScopedLock {
    MutexLock& lock;
public:
    ScopedLock(MutexLock& lock) : lock(lock) {
        lock.acquireLock();
    }
    ~ScopedLock() {
        lock.releaseLock();
    }
};
