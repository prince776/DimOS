#pragma once

#include <kernel/memory/vmm.h>

namespace Heap {

    struct __attribute__((packed)) Node {
        uint32_t size;
        Node* next = nullptr;
    };

    constexpr int sizeofNode = sizeof(Node);

    class FreeList {
        Node* head;
    public:
        FreeList(VirtualAddr addr, uint32_t size);
        void* alloc(uint32_t size);
        void addMemory(VirtualAddr addr, uint32_t size);
        void free(void* ptr);
    private:
        // returns the other part of memory
        Node* split(Node* node, uint32_t size);
        void addNode(Node* newNode);
    };
};
