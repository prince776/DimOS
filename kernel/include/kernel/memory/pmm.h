#pragma once
#include <stdio.h>
#include <stdint-gcc.h>

using PhysicalAddr = uint64_t;

struct MemRange {
    uint64_t start, size;
};

struct PhysicalMemMap {
    PhysicalMemMap() {
        totalSize = availMemArrCnt = kernelStart = kernelSize = 0;
        availableMemArr = nullptr;
    }
    uint64_t totalSize;
    MemRange* availableMemArr;
    uint64_t availMemArrCnt;
    uint64_t kernelStart, kernelSize;
};


class PMM {
public:
    static constexpr int64_t framesPerByte = 8;
    static constexpr int64_t framesPerElement = 64;
    static constexpr int64_t elementSize = 8;
    static constexpr int64_t frameSize = 4096;
    static constexpr int64_t frameAlignment = 4096;

    uint64_t memorySize;
    int64_t usedframes, frameCnt;
    uint64_t* bitmap;

private:
    PMM() { memorySize = usedframes = frameCnt = 0; bitmap = nullptr; }
    PMM(const PMM&);
    void operator=(const PMM&);
public:
    static PMM& get() {
        static PMM pmm;
        return pmm;
    }

    // MemRanges in this map MUST be 4k aligned.
    // Returns the extra memory range used by pmm during initialization.
    MemRange init(const PhysicalMemMap& mem);

    inline void bitmapSet(int64_t bit) {
        bitmap[bit / framesPerElement] |= (1LL << (bit % framesPerElement));
    }

    inline void bitmapUnset(int64_t bit) {
        bitmap[bit / framesPerElement] &= ~(1LL << (bit % framesPerElement));
    }

    inline bool bitmapIsSet(int64_t bit) {
        return bitmap[bit / framesPerElement] & (1LL << (bit % framesPerElement));
    }

    inline int64_t elementCnt() {
        return (int64_t)(frameCnt + framesPerElement - 1) / framesPerElement;
    }

    inline PhysicalAddr getFrameAddr(int64_t frame) { return (frame << 12); }

    inline PhysicalAddr endMemory() { return (PhysicalAddr)bitmap + elementCnt(); }
    int64_t firstFreeFrame();
    int64_t firstNFreeFrames(int64_t n);

    void initRegion(PhysicalAddr start, uint64_t size);
    void deinitRegion(PhysicalAddr start, uint64_t size);

    PhysicalAddr allocFrame();
    PhysicalAddr allocNFrames(int64_t n);
    void freeFrame(PhysicalAddr addr);
};
