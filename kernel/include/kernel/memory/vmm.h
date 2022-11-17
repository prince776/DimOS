#pragma once
#include <stdio.h>
#include <stdint-gcc.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/paging.h>

using VirtualAddr = uint32_t;

class VMM {
    static constexpr int pagesPerTable = 1024;
    static constexpr int tablesPerDir = 1024;
    static constexpr int pageSize = 4096;
    static constexpr int pageAlignment = 4096;

    PageDirectory* dir;
    PMM& pmm;
public:
    // Enable paging and identity map the current used page frames
    VMM(PMM& pmm);

    inline int getPdIdx(VirtualAddr x) const { return (((x) >> 22) & 0x3ff); }
    inline int getPtIdx(VirtualAddr x) const { return (((x) >> 12) & 0x3ff); }

    // pd routines
    bool switchDir(PageDirectory* dir);
    inline PageDirectory* getDir() { return dir; }

    // pt routines
    // bool allocPage(PTEntry* entry);
    // bool freePage(PTEntry* entry);

};
