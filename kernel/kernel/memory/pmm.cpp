#include <kernel/memory/pmm.h>
#include <string.h>
#include <stdio.h>
#include <kernel/common.h>

#ifdef PMMLOG
#define LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define LOG(...)
#endif

extern uint64_t HHDMOffset;

void PMM::init(const PhysicalMemMap& mem) {
    memorySize = mem.totalSize;
    frameCnt = memorySize / frameSize;

    uint64_t reqMemForBitmap = elementCnt() * elementSize;
    MemRange memRangeforBitmap;
    for (int i = 0; i < 2; i++) {
        const auto& memRange = mem.availableMemArr[i];
        if (memRange.size >= reqMemForBitmap) {
            bitmap = (uint64_t*)(memRange.start + HHDMOffset);
            memRangeforBitmap.start = memRange.start;
            memRangeforBitmap.size = reqMemForBitmap;
            break;
        }
    }
    if (bitmap == nullptr) {
        panic("Can't allocate memory for PMM\n");
        return;
    }
    // All memory unavilable initially
    deinitRegion(0, memorySize);
    usedframes = frameCnt;

    // Initialize from mem map
    for (int i = 0; i < mem.availMemArrCnt; i++) {
        const auto& memRange = mem.availableMemArr[i];
        initRegion(memRange.start, memRange.size);
    }
    // Deintialize the memory used for storing bitmap
    deinitRegion(memRangeforBitmap.start, memRangeforBitmap.size);
    deinitRegion(0, frameSize);
}

void PMM::initRegion(PhysicalAddr start, uint64_t size) {
    LOG("[PMM] Initializing Physical memory region: [%u, %u)\n", start, start + size);

    int64_t frameSt = (start + frameSize - 1) / frameSize;
    int64_t remSize = size - (frameSt * frameSize - start);
    int64_t cnt = (remSize + frameSize - 1) / frameSize;

    while (cnt-- > 0) {
        bool wasSet = bitmapIsSet(frameSt);
        bitmapUnset(frameSt++);
        if (wasSet) {
            usedframes--;
        }
    }
}

void PMM::deinitRegion(PhysicalAddr start, uint64_t size) {
    LOG("[PMM] Deinitializing Physical memory region: [%u, %u)\n", start, start + size);

    int64_t frameSt = (start + frameSize - 1) / frameSize;
    int64_t remSize = size - (frameSt * frameSize - start);
    int64_t cnt = (remSize + frameSize - 1) / frameSize;

    while (cnt-- > 0) {
        bool wasUnset = !bitmapIsSet(frameSt);
        bitmapSet(frameSt++);
        if (wasUnset) {
            usedframes++;
        }
    }
}

int64_t PMM::firstFreeFrame() {
    for (int64_t i = 0; i < elementCnt(); i++) {
        if (bitmap[i] == 0xffffffffffffffff) continue;
        for (int64_t j = 0; j < 64; j++) {
            if (!(bitmap[i] & (1LL << j))) {
                return i * framesPerElement + j;
            }
        }
    }
    return -1;
}

int64_t PMM::firstNFreeFrames(int64_t n) {
    if (n > elementCnt()) return -1;
    int64_t free = 0, i = 0;
    for (; i < n; i++) {
        free += !bitmapIsSet(i);
    }
    for (; i < elementCnt() * framesPerElement; i++) {
        if (free == n) return i - n;
        free += !bitmapIsSet(i);
        free -= !bitmapIsSet(i - n);
    }
    if (free == n) return i - n;
    return -1;
}

PhysicalAddr PMM::allocFrame() {
    if (usedframes == frameCnt)
        return 0;
    auto frame = firstFreeFrame();
    if (frame == -1)
        return 0;

    LOG("[PMM] Allocating frame: %l\n", frame);
    bitmapSet(frame);
    usedframes++;
    PhysicalAddr addr = frame * frameSize;
    return addr;
}

PhysicalAddr PMM::allocNFrames(int64_t n) {
    if (usedframes + n >= frameCnt)
        return 0;
    auto frame = firstNFreeFrames(n);
    if (frame == -1)
        return 0;

    LOG("[PMM] Allocating %l frames starting at: %l\n", n, frame);
    for (int64_t i = 0; i < n; i++) {
        bitmapSet(frame + i);
        usedframes++;
    }
    PhysicalAddr addr = frame * frameSize;
    return addr;
}

void PMM::freeFrame(PhysicalAddr addr) {
    int64_t frame = addr / frameSize;
    LOG("Freeing frame: %l\n", frame);
    if (!bitmapIsSet(frame)) return;

    bitmapUnset(frame);
    usedframes--;
}
