#pragma once
#include <kernel/devices/pic.h>
#include <kernel/x64.h>
#include <stdint.h>

namespace pit {

constexpr int IRQLine = 0;

void init(uint16_t divisor);

void stop();

}  // namespace pit
