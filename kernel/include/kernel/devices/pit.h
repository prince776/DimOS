#pragma once
#include <stdint-gcc.h>
#include <kernel/devices/pic.h>
#include <kernel/x64.h>

namespace pit {

    constexpr int IRQLine = 0;

    void init(uint16_t divisor);

    void stop();

}