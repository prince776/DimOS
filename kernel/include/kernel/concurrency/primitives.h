#pragma once
#include <stdint-gcc.h>

class MutexLock {
private:
    int64_t flag = 0;
public:
    MutexLock() = default;

    void acquireLock() {
        while (__sync_bool_compare_and_swap(&flag, 0, 1) == 0)
            continue;
    }
    void releaseLock() {
        flag = 0;
    }
};
