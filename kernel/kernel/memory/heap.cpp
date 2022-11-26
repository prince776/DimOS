#include <kernel/memory/heap.h>
#include <stdio.h>

namespace Heap {

    FreeList::FreeList(VirtualAddr addr, uint32_t size) {
        addMemory(addr, size);
    }

    void* FreeList::alloc(uint32_t size) {
        Node* curr = head, * target = nullptr, * prev, * prevNode;
        int32_t usedSize = INT32_MAX;
        while (curr != nullptr) {
            if (curr->size >= size && curr->size < usedSize) {
                target = curr;
                prev = prevNode;
                usedSize = target->size;
            }
            prevNode = curr;
            curr = curr->next;
        }
        if (target == nullptr) return nullptr;
        Node* splitPart = split(target, size);
        if (splitPart != nullptr) {
            if (prev != nullptr) {
                prev->next = splitPart;
            }
            else {
                head = splitPart;
            }
            splitPart->next = target->next;
        }
        else {
            if (prev != nullptr) {
                prev->next = target->next;
            }
            else {
                head = target->next;
            }
        }
        return (void*)target + sizeofNode;
    }

    void FreeList::addNode(Node* newNode) {
        Node* curr = head, * prev = nullptr;
        while (curr != nullptr && curr <= newNode) {
            if ((uint32_t)curr + sizeofNode + curr->size == (uint32_t)newNode) { // it can be coalesced into curr
                curr->size += sizeofNode + newNode->size;
                Node* next = curr->next;
                if ((uint32_t)next == (uint32_t)curr + sizeofNode + curr->size) { // next can be coalesced into curr
                    curr->size += sizeofNode + next->size;
                    curr->next = next->next;
                }
                return;
            }
            prev = curr;
            curr = curr->next;
        }
        if (prev) {
            prev->next = newNode;
        }
        else {
            head = newNode;
        }
        newNode->next = curr;
    }

    void FreeList::addMemory(VirtualAddr addr, uint32_t size) {
        Node* newNode = (Node*)addr;
        newNode->size = size - sizeofNode;
        addNode(newNode);
    }

    void FreeList::free(void* ptr) {
        Node* newNode = (Node*)(ptr - sizeofNode);
        addNode(newNode);
    }

    Node* FreeList::split(Node* node, uint32_t size) {
        if (node->size < size + sizeofNode + 1) return nullptr;
        Node* splitPart = (Node*)((uint32_t)node + sizeofNode + size);
        splitPart->size = node->size - size - sizeofNode;
        return splitPart;
    }
}