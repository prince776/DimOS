#pragma once
#include <stdio.h>
#include <stdint-gcc.h>

using PhysicalAddr = uint32_t;

class PMM {
    static constexpr int framesPerByte = 8;
    static constexpr int framesPerElement = 32;
    static constexpr int frameSize = 4096;
    static constexpr int frameAlignment = 4096;

    uint32_t memorySize, usedframes, frameCnt;
    uint32_t* bitmap;

public:
    // kernel_end_mem MUST be 4K aligned
    PMM(uint32_t memSize, uint32_t* kernel_end_mem);

    inline void bitmapSet(int bit) {
        bitmap[bit / framesPerElement] |= (1 << (bit % framesPerElement));
    }

    inline void bitmapUnset(int bit) {
        bitmap[bit / framesPerElement] &= ~(1 << (bit % framesPerElement));
    }

    inline bool bitmapIsSet(int bit) {
        return bitmap[bit / framesPerElement] & (1 << (bit % framesPerElement));
    }

    inline int elementCnt() {
        return frameCnt / framesPerElement;
    }

    int firstFreeFrame();
    // int firstNFreeFrames(int n);

    void initRegion(PhysicalAddr start, uint32_t size);
    void deinitRegion(PhysicalAddr start, uint32_t size);

    void* allocFrame();
    void freeFrame(void* addr);
};
