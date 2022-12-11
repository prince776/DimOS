#pragma once

#include <kernel/memory/vmm.h>

namespace Heap {

    struct __attribute__((packed)) Node {
        uint64_t size;
        Node* next = nullptr;
        void print();
    };

    constexpr int64_t sizeofNode = sizeof(Node);

    class FreeList {
        Node* head;
    public:
        FreeList() { head = nullptr; }
        FreeList(VirtualAddr addr, uint64_t size);
        void* alloc(uint64_t size);
        void addMemory(VirtualAddr addr, uint64_t size);
        void free(void* ptr);
        void print();
    private:
        // returns the other part of memory
        Node* split(Node* node, uint64_t size);
        void addNode(Node* newNode);
    };
};

namespace MallocHeap {
    void init();
    Heap::FreeList get();
}
