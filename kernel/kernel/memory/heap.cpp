#include <kernel/memory/heap.h>
#include <stdio.h>

namespace Heap {

FreeList::FreeList(VirtualAddr addr, uint64_t size) {
    head = nullptr;
    addMemory(addr, size);
}

void* FreeList::alloc(uint64_t size) {
    Node *curr = head, *target = nullptr, *prev = nullptr, *prevNode = nullptr;
    uint64_t usedSize = UINT64_MAX;
    while (curr != nullptr) {
        if (curr->size >= size && curr->size < usedSize) {
            target = curr;
            prev = prevNode;
            usedSize = target->size;
        }
        prevNode = curr;
        curr = curr->next;
    }
    if (target == nullptr) {
        // ask for more memory from vmm
        int reqPages = (size + sizeofNode + VMM::pageSize - 1) / VMM::pageSize;
        VirtualAddr addr = VMM::get().allocNPages(true, reqPages); // TODO: Use LH for users
        addMemory(addr, reqPages * VMM::pageSize);
        return alloc(size); // retry
    }
    Node* splitPart = split(target, size);
    if (splitPart != nullptr) {
        if (prev != nullptr) {
            prev->next = splitPart;
        } else {
            head = splitPart;
        }
        splitPart->next = target->next;
        target->size = size;
    } else {
        if (prev != nullptr) {
            prev->next = target->next;
        } else {
            head = target->next;
        }
    }
    return (void*)target + sizeofNode;
}

void FreeList::addNode(Node* newNode) {
    Node *curr = head, *prev = nullptr;
    while (curr != nullptr && curr <= newNode) {
        // Note: This casting important because pointer arithmetic :p
        if ((uint64_t)curr + sizeofNode + curr->size ==
            (uint64_t)newNode) { // it can be coalesced into curr from front
            curr->size += sizeofNode + newNode->size;
            Node* next = curr->next;
            if ((uint64_t)next ==
                (uint64_t)curr + sizeofNode + curr->size) { // next can be coalesced into curr
                curr->size += sizeofNode + next->size;
                curr->next = next->next;
            }
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    newNode->next = curr;
    if (prev) {
        prev->next = newNode;
    } else {
        head = newNode;
    }
    if ((uint64_t)newNode + sizeofNode + newNode->size ==
        (uint64_t)curr) { // it can be coalesced into curr from back
        newNode->size += sizeofNode + curr->size;
        newNode->next = curr->next;
        Node* next = curr->next;
        if ((uint64_t)newNode + sizeofNode + newNode->size == (uint64_t)next) {
            newNode->size += sizeofNode + next->size;
            newNode->next = next->next;
        }
    }
}

void FreeList::addMemory(VirtualAddr addr, uint64_t size) {
    Node* newNode = (Node*)addr;
    newNode->size = size - sizeofNode;
    newNode->next = nullptr;
    addNode(newNode);
}

void FreeList::free(void* ptr) {
    Node* newNode = (Node*)(ptr - sizeofNode);
    addNode(newNode);
}

Node* FreeList::split(Node* node, uint64_t size) {
    if (node->size < size + sizeofNode + 1)
        return nullptr;
    Node* splitPart = (Node*)((uint64_t)node + sizeofNode + size);
    splitPart->size = node->size - size - sizeofNode;
    splitPart->next = nullptr;
    return splitPart;
}

void FreeList::print() {
    Node* curr = head;
    printf("------FREELIST----\n");
    while (curr != nullptr) {
        curr->print();
        curr = curr->next;
    }
    printf("------------------\n");
}

void Node::print() { printf("Node at: %x, sz: %x, next: %x\n", this, size, next); }
} // namespace Heap
