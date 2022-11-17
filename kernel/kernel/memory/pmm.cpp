#include <kernel/memory/pmm.h>
#include <string.h>
#include <stdio.h>

#ifdef PMMLOG
#define LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define LOG(...)
#endif

PMM::PMM(uint32_t memSize, uint32_t* kernel_end_mem, uint32_t ramEnd)
    : memorySize(memSize) {
    frameCnt = this->memorySize / frameSize;
    bitmap = kernel_end_mem;

    // all memory unavilable initially
    deinitRegion(0, memSize);
    // initialize memory from [bitmap_end, ramend)
    initRegion(endMemory(), ramEnd - endMemory());
}


void PMM::initRegion(PhysicalAddr start, uint32_t size) {
    LOG("[PMM] Initializing Physical memory region: [%u, %u)\n", start, start + size);

    int frameSt = (start + frameSize - 1) / frameSize;
    int cnt = (size + frameSize - 1) / frameSize;

    while (cnt-- > 0) {
        bool wasSet = bitmapIsSet(frameSt);
        bitmapUnset(frameSt++);
        if (wasSet) {
            usedframes--;
        }
    }
}

void PMM::deinitRegion(PhysicalAddr start, uint32_t size) {
    LOG("[PMM] Deinitializing Physical memory region: [%u, %u)\n", start, start + size);

    int frameSt = (start + frameSize - 1) / frameSize;
    int cnt = (size + frameSize - 1) / frameSize;

    while (cnt-- > 0) {
        bool wasUnset = !bitmapIsSet(frameSt);
        bitmapSet(frameSt++);
        if (wasUnset) {
            usedframes++;
        }
    }
}

int PMM::firstFreeFrame() {
    for (int i = 0; i < elementCnt(); i++) {
        if (bitmap[i] == 0xffffffff) continue;
        for (int j = 0; j < 32; j++) {
            if (!(bitmap[i] & (1 << j))) {
                return i * framesPerElement + j;
            }
        }
    }
    return -1;
}

void* PMM::allocFrame() {
    if (usedframes == frameCnt)
        return 0;
    int frame = firstFreeFrame();
    if (frame == -1)
        return 0;

    LOG("[PMM] Allocating frame: %d\n", frame);
    bitmapSet(frame);
    usedframes++;
    PhysicalAddr addr = frame * frameSize;
    return (void*)addr;
}

void PMM::freeFrame(void* addr) {
    PhysicalAddr paddr = (PhysicalAddr)addr;
    int frame = paddr / frameSize;

    if (bitmapIsSet(frame)) return;

    bitmapUnset(frame);
    usedframes--;
}
