#pragma once
#include <stdint-gcc.h>

struct Resource;

using readFn_t = uint32_t(*)(Resource* node, uint32_t offset, uint32_t size, uint8_t* buffer);
using writeFn_t = uint32_t(*)(Resource* node, uint32_t offset, uint32_t size, uint8_t* buffer);

struct Resource {
    int32_t size{};
    readFn_t readFn{};
    writeFn_t writeFn_t{};
};

