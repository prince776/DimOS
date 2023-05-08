#pragma once
#include <kernel/process/kthread.h>
#include <stdint.h>

#include <kernel/cpp/vector.hpp>

class MutexLock {
  private:
    // flag is the mutex value, lock is guard to access flag and queue
    int64_t flag = 0, lock = 0;
    Vector<uint64_t> queue; // a vector named queue that will be used as stack
  public:
    MutexLock() = default;

    void acquireLock();
    void releaseLock();
};

class ConditionVariable {
  private:
    int64_t lock = 0;
    Vector<uint64_t> queue; // a vector named queue that will be used as stack
  public:
    ConditionVariable() = default;

    // PreCondition: This mutex lock must be held
    void wait(MutexLock& mutex);

    void notifyOne();
    void notifyAll();
};

// Intentionally made using Mutex and ConditionVariable, so that if
// there's a big in concurrency, only they need to be fixed.
class Sempahore {
  private:
    int32_t value;
    MutexLock lock;
    ConditionVariable condition;

  public:
    Sempahore(int value) : value(value) {}

    void wait();
    void post();
};
