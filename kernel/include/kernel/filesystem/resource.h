#pragma once
#include <stdint.h>

struct Resource;

struct Resource {
    int32_t inode{};
    int32_t size{};
    int32_t deviceID{};
};
