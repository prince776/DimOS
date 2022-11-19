#include <kernel/memory/vmm.h>
#include <kernel/memory/paging.h>
#include <kernel/common.h>
#include <kernel/isr.h>

extern "C" void loadPageDirectory(unsigned int*);
extern "C" void enablePaging();

VMM::VMM(PMM& pmm)
    : pmm(pmm) {
    dir = (PageDirectory*)pmm.allocNFrames(2);
    (*dir).Init();

    // ASSUMPTION: only first 4 MB of available ram is used till now
    // If more, then we'll have to allocate more pages
    PageTable* pt0 = (PageTable*)pmm.allocFrame();
    (*pt0).Init();
    freePages += pagesPerTable;
    dir->vAddrs[0] = (VirtualAddr)pt0; // since identity mapped
    pde::setFrame(&dir->entries[0], (PhysicalAddr)pt0);
    pde::addAttrib(&dir->entries[0], I86_PDE_PRESENT);
    pde::addAttrib(&dir->entries[0], I86_PDE_WRITABLE);

    // Create identity mapping
    for (int frame = 0; frame < PageTable::entryCnt; frame++) // Map the first page table
    {
        if (!pmm.bitmapIsSet(frame)) continue; // skip unused frames
        PhysicalAddr pAddr = pmm.getFrameAddr(frame);
        VirtualAddr vAddr = pAddr; // Identity mapping
        int pdIdx = getPdIdx(vAddr);
        int ptIdx = getPtIdx(vAddr);

        PDEntry pde = dir->entries[pdIdx];
        PageTable* pt = (PageTable*)pde::getPhysicalAddr(pde);

        pte::setFrame(&pt->entries[ptIdx], pAddr);
        pte::addAttrib(&pt->entries[ptIdx], I86_PTE_PRESENT);
        pte::addAttrib(&pt->entries[ptIdx], I86_PTE_WRITABLE);

        freePages--;
    }

    switchDir(dir);
    enablePaging();
    register_interrupt_handler(14, page_fault_handler);
}

bool VMM::switchDir(PageDirectory* dir) {
    if (!dir)
        return false;
    this->dir = dir;
    loadPageDirectory((unsigned int*)this->dir);
    return true;
}

void* VMM::allocPage() {
    PageInfo freePage = firstFreePage();
    if (!freePage.pte) {
        panic("OUT OF PHYSICAL MEMORY");
    }
    PhysicalAddr freeFrame = (PhysicalAddr)pmm.allocFrame();

    if (freePages == 1) { // can't use this one up, otherwise we can't allocate a new page for storing next page table
        assignFrameToPage(freePage.pte, freeFrame);

        // Create a new page table in this free page
        PageTable* pt = (PageTable*)freePage.getVirtualAddr();
        (*pt).Init();
        freePages += pagesPerTable;

        int pdIdx = 0;
        for (; pdIdx < PageDirectory::entryCnt; pdIdx++) {
            if (pde::isPresent(dir->entries[pdIdx])) continue;
        }
        dir->vAddrs[pdIdx] = (VirtualAddr)pt;
        pde::setFrame(&dir->entries[pdIdx], freeFrame);
        pde::addAttrib(&dir->entries[0], I86_PDE_PRESENT);
        pde::addAttrib(&dir->entries[0], I86_PDE_WRITABLE);

        // Now restart the routine, we have enough pages now
        return allocPage();
    }

    assignFrameToPage(freePage.pte, freeFrame);
    return (void*)freePage.getVirtualAddr();
}

void VMM::assignFrameToPage(PTEntry* e, PhysicalAddr frameAddr) {
    pte::setFrame(e, frameAddr);
    pte::addAttrib(e, I86_PTE_PRESENT);
    pte::addAttrib(e, I86_PTE_WRITABLE);
    freePages--;
}

PageInfo VMM::firstFreePage() {
    if (freePages <= 0) { return PageInfo{ 0, 0, 0 }; }

    // check for usable PTEntry in available tables
    for (int i = 0; i < PageDirectory::entryCnt; i++) {
        PDEntry* pde = &dir->entries[i];

        if (!pde::isPresent(*pde)) continue;

        PageTable* pt = (PageTable*)dir->vAddrs[i];
        for (int j = 0; j < PageTable::entryCnt; j++) {
            PTEntry* pte = &pt->entries[j];
            if (pte::isPresent(*pte)) continue;
            return PageInfo{ pte, i, j };
        }
    }
    return PageInfo{ 0, 0, 0 };
}

void VMM::freePage(void* ptr) {
    VirtualAddr vAddr = (VirtualAddr)ptr;

    PageInfo page;
    page.pdIdx = getPdIdx(vAddr);
    page.ptIdx = getPtIdx(vAddr);

    PageTable* pt = (PageTable*)dir->vAddrs[page.pdIdx];
    page.pte = (PTEntry*)&pt->entries[page.ptIdx];

    pmm.freeFrame((void*)pte::getPhysicalAddr(*page.pte));
    *page.pte = pte::defaultVal;
    freePages++;
    invlpg(page.getVirtualAddr());
}