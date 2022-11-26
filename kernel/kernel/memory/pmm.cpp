#include <kernel/memory/pmm.h>
#include <string.h>
#include <stdio.h>

#ifdef PMMLOG
#define LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define LOG(...)
#endif

void PMM::init(uint32_t memSize, uint32_t* kernel_end_mem, uint32_t ramEnd) {
    memorySize = memSize;
    frameCnt = memorySize / frameSize;
    bitmap = kernel_end_mem;

    // all memory unavilable initially
    deinitRegion(0, memorySize);
    usedframes = frameCnt;
    // initialize memory from [bitmap_end, ramend)
    initRegion((PhysicalAddr)endMemory(), ramEnd - endMemory());
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

int PMM::firstNFreeFrames(int n) {
    if (n > elementCnt()) return -1;
    int free = 0, i = 0;
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

void* PMM::allocNFrames(int n) {
    if (usedframes + n >= frameCnt)
        return 0;
    int frame = firstNFreeFrames(n);
    if (frame == -1)
        return 0;

    LOG("[PMM] Allocating %d frames starting at: %d\n", n, frame);
    for (int i = 0; i < n; i++) {
        bitmapSet(frame + i);
        usedframes++;
    }
    PhysicalAddr addr = frame * frameSize;
    return (void*)addr;
}

void PMM::freeFrame(void* addr) {
    PhysicalAddr paddr = (PhysicalAddr)addr;
    int frame = paddr / frameSize;
    LOG("Freeing frame: %d\n", frame);
    if (!bitmapIsSet(frame)) return;

    bitmapUnset(frame);
    usedframes--;
}
