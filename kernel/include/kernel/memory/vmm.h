#pragma once
#include <stdio.h>
#include <stdint-gcc.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/paging.h>


struct PageInfo {
    PTEntry* pte;
    int pdIdx, ptIdx;

    VirtualAddr getVirtualAddr() const {
        return ((pdIdx << 22) | (ptIdx << 12));
    }
};

class VMM {
public:
    static constexpr int pagesPerTable = 1024;
    static constexpr int tablesPerDir = 1024;
    static constexpr int pageSize = 4096;
    static constexpr int pageAlignment = 4096;

private:
    PageDirectory* dir;
    int freePages = 0; // with every new page table added, freePages += pagesPerTable, with every page allocated, freePages-- 

private:
    VMM() {}
    VMM(const VMM&);
    void operator=(const VMM&);

public:
    static VMM& get() {
        static VMM vmm;
        return vmm;
    }
    // Enable paging and identity map the current used page frames
    void init();

    inline int getPdIdx(VirtualAddr x) const { return (((x) >> 22) & 0x3ff); }
    inline int getPtIdx(VirtualAddr x) const { return (((x) >> 12) & 0x3ff); }

    // pd routines
    bool switchDir(PageDirectory* dir);
    inline PageDirectory* getDir() { return dir; }

    // pt routines
    void* allocPage();
    void* allocNPages(int n);
    void assignFrameToPage(PTEntry* entry, PhysicalAddr frameAddr);
    void freePage(void* ptr);

    PageInfo firstFreePage();

    inline void invlpg(VirtualAddr addr) {
        asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
    }

};
