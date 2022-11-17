#include <kernel/memory/vmm.h>
#include <kernel/memory/paging.h>

extern "C" void loadPageDirectory(unsigned int*);
extern "C" void enablePaging();

VMM::VMM(PMM& pmm)
    : pmm(pmm) {
    dir = (PageDirectory*)pmm.allocFrame();

    // ASSUMPTION: only first 4 MB of available ram is used till now
    // If more, then we'll have to allocate more pages
    PageTable* pt0 = (PageTable*)pmm.allocFrame();
    pde::setFrame(&dir->entries[0], (PhysicalAddr)pt0);
    pde::addAttrib(&dir->entries[0], I86_PDE_PRESENT);
    pde::addAttrib(&dir->entries[0], I86_PDE_WRITABLE);

    // Create identity mapping
    for (int frame = 0; frame < PageTable::entryCnt; frame++) // Map the first page table
    {
        // if (!pmm.bitmapIsSet(frame)) continue; // skip unused frames
        PhysicalAddr pAddr = pmm.getFrameAddr(frame);
        VirtualAddr vAddr = pAddr; // Identity mapping
        int pdIdx = getPdIdx(vAddr);
        int ptIdx = getPtIdx(vAddr);

        PDEntry pde = dir->entries[pdIdx];
        PageTable* pt = (PageTable*)pde::getPhysicalAddr(pde);

        pte::setFrame(&pt->entries[ptIdx], pAddr);
        pte::addAttrib(&pt->entries[ptIdx], I86_PTE_PRESENT);
        pte::addAttrib(&pt->entries[ptIdx], I86_PTE_WRITABLE);
    }

    switchDir(dir);
    enablePaging();
}

bool VMM::switchDir(PageDirectory* dir) {
    if (!dir)
        return false;
    this->dir = dir;
    loadPageDirectory((unsigned int*)this->dir);
    return true;
}

// bool VMM::allocPage(PTEntry* entry) {
//     void* p = pmm.allocFrame();
//     if (!p)
//         return false;
//     return true;
// }
