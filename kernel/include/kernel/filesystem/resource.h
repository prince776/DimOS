#pragma once
#include <stdint-gcc.h>

struct Resource;

struct Stat {
    int32_t inode{};
    int32_t size{};
    int32_t deviceID{};
};

struct Resource {
    Stat stat{};
};


