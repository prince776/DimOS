#include <kernel/memory/heap.h>
#include <kernel/memory/kheap.h>

static Heap::FreeList mallocHeap;

namespace MallocHeap {
void init() {
    VirtualAddr firstHeapPage = VMM::get().allocPage(true);
    mallocHeap = Heap::FreeList(firstHeapPage, VMM::pageSize);
}

Heap::FreeList get() { return mallocHeap; }
} // namespace MallocHeap

extern "C" void* kmalloc(uint64_t size) {
    auto res = mallocHeap.alloc(size);
    return res;
}
extern "C" void kfree(void* ptr) { return mallocHeap.free(ptr); }
