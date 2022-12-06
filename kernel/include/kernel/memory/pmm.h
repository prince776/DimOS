#pragma once
#include <stdio.h>
#include <stdint-gcc.h>

using PhysicalAddr = uint32_t;

class PMM {
public:
    static constexpr int framesPerByte = 8;
    static constexpr int framesPerElement = 32;
    static constexpr int frameSize = 4096;
    static constexpr int frameAlignment = 4096;

    uint32_t memorySize, usedframes, frameCnt;
    uint32_t* bitmap;

private:
    PMM() {}
    PMM(const PMM&);
    void operator=(const PMM&);
public:
    static PMM& get() {
        static PMM pmm;
        return pmm;
    }

    // kernel_end_mem MUST be 4K aligned
    void init(uint32_t memSize, uint32_t* kernel_end_mem, uint32_t ramEnd);

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

    inline PhysicalAddr getFrameAddr(int frame) { return (frame << 12); }

    inline PhysicalAddr endMemory() { return 0; }//}(PhysicalAddr)bitmap + elementCnt(); }
    int firstFreeFrame();
    int firstNFreeFrames(int n);

    void initRegion(PhysicalAddr start, uint32_t size);
    void deinitRegion(PhysicalAddr start, uint32_t size);

    void* allocFrame();
    void* allocNFrames(int n);
    void freeFrame(void* addr);
};
