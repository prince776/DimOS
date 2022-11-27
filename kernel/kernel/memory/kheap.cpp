#include <kernel/memory/kheap.h>
#include <kernel/memory/heap.h>

static Heap::FreeList mallocHeap;

namespace MallocHeap {
    void init() {
        void* firstHeapPage = VMM::get().allocPage();
        mallocHeap = Heap::FreeList((VirtualAddr)firstHeapPage, VMM::pageSize);
    }

    Heap::FreeList get() {
        return mallocHeap;
    }
}

extern "C" void* kmalloc(uint32_t size) {
    return mallocHeap.alloc(size);
}
extern "C" void kfree(void* ptr) {
    return mallocHeap.free(ptr);
}
