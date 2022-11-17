#pragma once

#include <stdint-gcc.h>
#include <kernel/memory/pmm.h>

//////////////////////////////////////// Page Table Entry ////////////////////////////////////////

enum PTEAttrib {
    I86_PTE_PRESENT = 1,			//0000000000000000000000000000001
    I86_PTE_WRITABLE = 2,			//0000000000000000000000000000010
    I86_PTE_USER = 4,			//0000000000000000000000000000100
    I86_PTE_WRITETHOUGH = 8,			//0000000000000000000000000001000
    I86_PTE_NOT_CACHEABLE = 0x10,		//0000000000000000000000000010000
    I86_PTE_ACCESSED = 0x20,		//0000000000000000000000000100000 bnm,  
    I86_PTE_DIRTY = 0x40,		//0000000000000000000000001000000
    I86_PTE_PAT = 0x80,		//0000000000000000000000010000000
    I86_PTE_CPU_GLOBAL = 0x100,		//0000000000000000000000100000000
    I86_PTE_LV4_GLOBAL = 0x200,		//0000000000000000000001000000000
    I86_PTE_FRAME = 0x7FFFF000 	//1111111111111111111000000000000
};
using PTEntry = uint32_t;

namespace pte {
    static PTEntry defaultVal = 0;
    inline void addAttrib(PTEntry* e, PTEAttrib attrib) { *e |= attrib; }
    inline void delAttrib(PTEntry* e, PTEAttrib attrib) { *e &= ~attrib; }
    inline void setFrame(PTEntry* e, PhysicalAddr addr) { *e = (*e & ~I86_PTE_FRAME) | addr; }
    inline bool isPresent(PTEntry e) { return e & I86_PTE_PRESENT; }
    inline bool isWriteable(PTEntry e) { return e & I86_PTE_WRITABLE; }
    inline PhysicalAddr getPhysicalAddr(PTEntry e) { return e & I86_PTE_FRAME; }
};

//////////////////////////////////////// Page Directory Entry ////////////////////////////////////////

enum PDEAttrib {
    I86_PDE_PRESENT = 1,			//0000000000000000000000000000001
    I86_PDE_WRITABLE = 2,			//0000000000000000000000000000010
    I86_PDE_USER = 4,			//0000000000000000000000000000100
    I86_PDE_PWT = 8,			//0000000000000000000000000001000
    I86_PDE_PCD = 0x10,		//0000000000000000000000000010000
    I86_PDE_ACCESSED = 0x20,		//0000000000000000000000000100000
    I86_PDE_DIRTY = 0x40,		//0000000000000000000000001000000
    I86_PDE_4MB = 0x80,		//0000000000000000000000010000000
    I86_PDE_CPU_GLOBAL = 0x100,		//0000000000000000000000100000000
    I86_PDE_LV4_GLOBAL = 0x200,		//0000000000000000000001000000000
    I86_PDE_FRAME = 0x7FFFF000 	//1111111111111111111000000000000
};
using PDEntry = uint32_t;

namespace pde {
    static PDEntry defaultVal = 0;
    inline void addAttrib(PDEntry* e, PDEAttrib attrib) { *e |= attrib; }
    inline void delAttrib(PDEntry* e, PDEAttrib attrib) { *e &= ~attrib; }
    inline void setFrame(PDEntry* e, PhysicalAddr addr) { *e = (*e & ~I86_PDE_FRAME) | addr; }
    inline bool isPresent(PDEntry e) { return e & I86_PDE_PRESENT; }
    inline bool isWriteable(PDEntry e) { return e & I86_PDE_WRITABLE; }
    inline PhysicalAddr getPhysicalAddr(PTEntry e) { return e & I86_PDE_FRAME; }
    inline bool isUser(PDEntry e) { return e & I86_PDE_USER; }
    inline bool is4Mb(PDEntry e) { return e & I86_PDE_4MB; }
};

//////////////////////////////////////// PAGE TABLE ////////////////////////////////////////

struct PageTable {
    inline static constexpr int entryCnt = 1024;
    PTEntry entries[entryCnt];

    PageTable() {
        for (int i = 0; i < entryCnt; i++)
            entries[i] = pte::defaultVal;
    }
};

//////////////////////////////////////// PAGE DIRECTORY ////////////////////////////////////////

struct PageDirectory {
    inline static constexpr int entryCnt = 1024;
    PDEntry entries[entryCnt];

    PageDirectory() {
        for (int i = 0; i < entryCnt; i++)
            entries[i] = pde::defaultVal;
    }
};
